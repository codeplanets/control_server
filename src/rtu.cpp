#include <unistd.h>
#include <signal.h>

#include "rtu.h"
#include "packetizer.h"
#include "socketexception.h"

void rtu_timeout_handler(int sig) {
    if (sig == SIGALRM) {
        syslog(LOG_WARNING, "Timeout %d seconds", WAITING_SEC);
    }
    exit(EXIT_FAILURE);
}

namespace core {
    RTUclient::RTUclient(ServerSocket& sock)
    : Client(sock) {
        
    }

    RTUclient::~RTUclient() {
        mq.close();
        updateStatus(false);
    }

    void RTUclient::init(InitReq &msg) {
        this->rtuAddr = msg.fromAddr;
        this->serverAddr = msg.toAddr;
        this->scode = msg.siteCode;

        syslog(LOG_DEBUG, "RTUclient::init : RTU-0x%02X Server-0x%02X Sitecode-%s", this->rtuAddr.getAddr(), this->serverAddr.getAddr(), this->scode.getSiteCode());
    }

    int RTUclient::reqMessage(DATA* buf, DATA cmd) {
        // INIT_RES, HEART_BEAT_ACK, COMMAND_RTU
        if (cmd == INIT_RES) {
            InitRes msg;
            syslog(LOG_DEBUG, "RTUclient::reqMessage : InitRes size = %ld", sizeof(msg));

            string addr = find_rtu_addr(this->scode);
            if (addr != NOT_FOUND) {
                this->rtuAddr.setAddr(addr, RTU_ADDRESS);
            } else {
                syslog(LOG_WARNING, "Unknown Site Code from RTU. : %s", this->scode.getSiteCode());
                this->rtuAddr.setAddr(0x0000);
            }

            // Packetizer
            msg.fromAddr = this->rtuAddr;
            msg.toAddr = this->serverAddr;
            msg.siteCode = this->scode;
            msg.rtuAddr = this->rtuAddr;
            msg.crc8.setCRC8(common::calcCRC((DATA*)&msg, sizeof(msg)));
            msg.print();

            memcpy(buf, (char*)&msg, sizeof(msg));
            common::print_hex(buf, sizeof(msg));
            return sizeof(msg);

        } else if (cmd == HEART_BEAT_ACK) {
            HeartBeatAck msg;
            syslog(LOG_DEBUG, "RTUclient::reqMessage : Heartbeat Ack size = %ld", sizeof(msg));
            
            msg.fromAddr = this->rtuAddr;
            msg.toAddr = this->serverAddr;

            msg.crc8.setCRC8(common::calcCRC((DATA*)&msg, sizeof(msg)));
            msg.print();

            memcpy(buf, (char*)&msg, sizeof(msg));
            common::print_hex(buf, sizeof(msg));
            return sizeof(msg);

        } else if (cmd == COMMAND_RTU) {
            CommandRtu msg;
            syslog(LOG_DEBUG, "RTUclient::reqMessage : Command RTU size = %ld", sizeof(msg));

            msg.crc8.setCRC8(common::calcCRC((DATA*)&msg, sizeof(msg)));
            msg.print();

            memcpy(buf, (char*)&msg, sizeof(msg));
            common::print_hex(buf, sizeof(msg));
            return sizeof(msg);

        } else {
            syslog(LOG_WARNING, "Unknown message type. : 0x%X", cmd);
        }
        return 0;
    }
    
    void RTUclient::setTimeout() {
        struct sigaction action;
        action.sa_handler = rtu_timeout_handler;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        sigaction(SIGALRM, &action, NULL);
        alarm(WAITING_SEC);
    }

    void RTUclient::run() {
        
        DATA sock_buf[MAX_RAW_BUFF] = {0x00,};
        DATA mq_buf[MQ_MSGSIZE] = {0x00,};
        DATA sendbuf[MAX_RAW_BUFF] = {0x00, };

        int len = reqMessage(sendbuf, INIT_RES);
        newSock.send(sendbuf, len);

        if (isSiteCodeAvailable() == false) {
            common::sleep(5000);
            return;
        }
        createMessageQueue(RTU_MQ_NAME);
        updateStatus(true);

        u_short addr = this->rtuAddr.getAddr();
        pid_t pid = getpid();
        if (addr > 0) {
            read_mapper(RTU_DATA, mapper_list);
            int line = getTotalLine(RTU_DATA);
            mapper_list[line] = add_mapper(pid, addr);
            write_mapper(RTU_DATA, mapper_list);
            print_mapper(mapper_list);
        }

        while (true) {
            common::sleep(100);

            // TODO : Command Client Message Queue
            try {
                errno = 0;
                int rcvByte = mq.recv(mq_buf, sizeof(mq_buf));
                if (rcvByte > 0) {
                    if (mq_buf[0] == STX) {
                        if (mq_buf[1] == COMMAND_RTU) {  // Client
                            syslog(LOG_INFO, "MQ >> RTU : Command RTU.");
                            common::print_hex(mq_buf, rcvByte);

                            insertDatabase(true);
                            newSock.send(mq_buf, rcvByte);
                        } else {
                            syslog(LOG_WARNING, "Unknown message type from mq. : 0x%X", mq_buf[1]);
                        }
                    }else {
                        syslog(LOG_WARNING, "Error Start of Text from mq. : 0x%X", mq_buf[0]);
                    }
                }
            // sleep(1);
            } catch (exception& e) {
                syslog(LOG_CRIT, "[Error : %s:%d] Exception was caught : %s",__FILE__, __LINE__, e.what());
            }

            try {
                int msgSize = 0;
                int sendByte = 0;
                errno = 0;
                int rcvByte = newSock.peek(sock_buf, MAX_RAW_BUFF);
                if (rcvByte < 0) {
                    if (errno == EAGAIN) {
                        common::sleep(100);
                        continue;
                    } else {
                        syslog(LOG_ERR, "RTUclient::run : %s", strerror(errno));
                        break;
                    }
                } else if (rcvByte == 0) {
                    common::sleep(100);
                    cout << ".";
                    continue;
                } else {
                    common::print_hex(sock_buf, rcvByte);
                    if (sock_buf[0] == STX) {
                        MsgHeader head;
                        memcpy((void*)&head, sock_buf, sizeof(head));
                        head.print();

                        msgSize = sizeof(head) + head.length + sizeof(MsgTail);
                        // DATA msg_buf[MAX_RAW_BUFF] = {0x00,};
                        // memcpy((void*)msg_buf, sock_buf, msgSize);
                        common::print_hex(sock_buf, msgSize);
                    } else {
                        msgSize = rcvByte;
                    }
                }

                rcvByte = newSock.recv(sock_buf, msgSize);
                if (sock_buf[0] == STX) {
                    if (sock_buf[1] == INIT_REQ) {  // RTUs
                        syslog(LOG_INFO, "RTU >> SVR : RTU Init Req.");
                        alarm(WAITING_SEC);

                        InitReq msg;
                        assert(rcvByte == sizeof(msg));
                        memcpy((void*)&msg, sock_buf, rcvByte);
                        if (common::checkCRC((DATA*)&msg, rcvByte, msg.crc8.getCRC8()) == false) {
                            syslog(LOG_WARNING, "CRC Check Failed. : 0x%02X != 0x%02X", common::calcCRC((DATA*)&msg, rcvByte), msg.crc8.getCRC8());
                        }
                        msg.print();

                        syslog(LOG_INFO, "SVR >> RTU : RTU Init Res.");
                        if (isSiteCodeAvailable() == false) {
                            sendByte = reqMessage(sendbuf, INIT_RES);
                            newSock.send(sendbuf, sendByte);
                            common::sleep(5000);
                            return;
                        }
                        sendByte = reqMessage(sendbuf, INIT_RES);
                        newSock.send(sendbuf, sendByte);

                    } else if (sock_buf[1] == HEART_BEAT) {  // RTU
                        syslog(LOG_INFO, "RTU >> SVR : Heartbeat.");
                        alarm(WAITING_SEC);
                        HeartBeat msg;
                        assert(rcvByte == sizeof(msg));
                        memcpy((void*)&msg, sock_buf, rcvByte);
                        if (common::checkCRC((DATA*)&msg, rcvByte, msg.crc8.getCRC8()) == false) {
                            syslog(LOG_WARNING, "CRC Check Failed. : 0x%02X != 0x%02X", common::calcCRC((DATA*)&msg, rcvByte), msg.crc8.getCRC8());
                        }
                        msg.print();

                        syslog(LOG_INFO, "SVR >> RTU : Heartbeat Ack.");
                        sendByte = reqMessage(sendbuf, HEART_BEAT_ACK);
                        newSock.send(sendbuf, sendByte);

                    } else if (sock_buf[1] == COMMAND_RTU_ACK) {  // Client
                        syslog(LOG_INFO, "RTU >> SVR : Command RTU Ack.");
                        CommandRtuAck msg;
                        assert(rcvByte == sizeof(msg));
                        memcpy((void*)&msg, sock_buf, rcvByte);
                        if (common::checkCRC((DATA*)&msg, rcvByte, msg.crc8.getCRC8()) == false) {
                            syslog(LOG_WARNING, "CRC Check Failed. : 0x%02X != 0x%02X", common::calcCRC((DATA*)&msg, rcvByte), msg.crc8.getCRC8());
                        }
                        msg.print();

                        // TODO : Client MQ 에 데이터를 전송
                        Mq cmd_mq;
                        pid_t pid = 0;
                        std::vector<pid_t> pids;
                        // data에서 pid를 read.
                        core::common::MAPPER cmd_mapper_list[MAX_POOL] = {0, };
                        read_mapper(CLIENT_DATA, cmd_mapper_list);
                        int line = getTotalLine(CLIENT_DATA);
                        core::common::MAPPER* map;
                        for (map = cmd_mapper_list; map < cmd_mapper_list + line; map++) {
                            if (map->pid != 0) {
                                if (cmd_mq.open(CLIENT_MQ_NAME, map->pid)) {
                                    syslog(LOG_INFO, "SVR >> MQ : Command RTU Ack. : %d", map->pid);
                                    cmd_mq.send(sock_buf, sizeof(msg));
                                    cmd_mq.close();
                                } else {
                                    syslog(LOG_WARNING, "Failed to open Client MQ. : %d", pid);                                    
                                }
                            }
                        }
                        // core::common::MAPPER cmd_mapper_list[MAX_POOL] = {0, };
                        // read_mapper(CLIENT_DATA, cmd_mapper_list);
                        // int line = getTotalLine(CLIENT_DATA);
                        // search_mapper(cmd_mapper_list, pids, line, msg.toAddr.getAddr());
                        // for (auto it = pids.begin(); it!= pids.end(); it++) {
                        //     cout << *it << endl;
                        //     pid = *it;
                        //     if (pid != 0) {
                        //         cmd_mq.open(CLIENT_MQ_NAME, pid);
                        //         cmd_mq.send(sock_buf, sizeof(msg));
                        //         cmd_mq.close();
                        //     }
                        // }
                        // TODO : updateDatabase();

                    } else {
                        syslog(LOG_WARNING, "Unknown message type from socket.");
                    }
                } else {
                    syslog(LOG_WARNING, "Error Start of Text from socket.");
                }
            } catch (SocketException& se) {
                syslog(LOG_CRIT, "[Error : %s:%d] Exception was caught : [%d] %s",__FILE__, __LINE__, se.code(), se.description().c_str());
            }

        }
    }
    
    /**
     * @return true if SiteCode is available, false otherwise
    */
    bool RTUclient::isSiteCodeAvailable() {
        string siteCode = find_rtu_addr(this->scode);
        if ( siteCode == NOT_FOUND) {
            return false;
        }
        syslog(LOG_DEBUG, "Site Code is available. : %s", siteCode.c_str());
        return true;
    }

    core::common::MAPPER RTUclient::add_mapper(int pid, u_short addr) {
        core::common::MAPPER mapper;
        mapper.pid = pid;
        mapper.addr = addr;
        return mapper;
    }

    void RTUclient::print_mapper(core::common::MAPPER* mapper) {
        for (int i = 0; i < MAX_POOL; i++) {
            if (mapper[i].pid != 0) {
                syslog(LOG_DEBUG, "Mapper : %d 0x%02X", mapper[i].pid, mapper[i].addr);
            }
        }
    }

    void RTUclient::search_mapper(core::common::MAPPER* mapper, pid_t &pid, int idx, u_short addr) {
        core::common::MAPPER* map;
        for (map = mapper; map < mapper + idx; map++) {
            if (map->addr == addr) {
                syslog(LOG_DEBUG, "Found Mapper : %d 0x%02X", map->pid, map->addr);

                pid = map->pid;
            }
        }
    }

    void RTUclient::search_mapper(core::common::MAPPER* mapper, std::vector<pid_t> &pids, int idx, u_short addr) {
        core::common::MAPPER* map;
        for (map = mapper; map < mapper + idx; map++) {
            if (map->addr == addr) {
                syslog(LOG_DEBUG, "Found Mapper : %d 0x%02X", map->pid, map->addr);

                pids.push_back(map->pid);
            }
        }
    }

    bool RTUclient::delete_mapper(core::common::MAPPER* mapper, int idx, int pid) {
        bool ret = false;
        core::common::MAPPER* map;
        for (map = mapper; map < mapper + idx; map++) {
            if (map->pid == pid) {
                map->pid = 0;
                ret = true;
            }
        }
        return ret;
    }

    void RTUclient::write_mapper(std::string filename, core::common::MAPPER* mapper) {
        FILE * f = fopen(filename.c_str(), "a");
        for (int i = 0; i < MAX_POOL; i++) {
            if (mapper[i].pid != 0) {
                fprintf(f, "%d %hd\n", mapper[i].pid, mapper[i].addr);
            }
        }
        fclose(f);
    }

    void RTUclient::read_mapper(std::string filename, core::common::MAPPER* mapper) {
        FILE * f = fopen(filename.c_str(), "r");
        // core::common::MAPPER compare;
        for (int i = 0; i < MAX_POOL; i++) {
            fscanf(f, "%d %hd", &mapper[i].pid, &mapper[i].addr);
            // fscanf(f, "%d %hd", &compare.pid, &compare.addr);
            
        }
        fclose(f);
    }

    int RTUclient::getTotalLine(string name) {
        FILE *fp;
        int line=0;
        char c;
        fp = fopen(name.c_str(), "r");
        while ((c = fgetc(fp)) != EOF) {
            if (c == '\n') {
                line++;
            }
        }
        fclose(fp);
        return(line);
    }

    std::string RTUclient::find_rtu_addr(SiteCode scode) {
        string addr = NOT_FOUND;
        Database db;
        ECODE ecode = db.db_init("localhost", 3306, "rcontrol", "rcontrol2024", "RControl");
        if (ecode!= EC_SUCCESS) {
            syslog(LOG_ERR, "DB Connection Error!");
            exit(EXIT_FAILURE);
        }
        char* siteCode = scode.getSiteCode();
        string query = "select * from RSite";
        query += " where SiteCode = '";
        query += siteCode;
        query += "';";
        syslog(LOG_DEBUG, "Query : %s", query.c_str());
        delete[] siteCode;

        // Query Data
        MYSQL_ROW sqlrow;
        MYSQL_RES* pRes;
        ecode = db.db_query(query.c_str(), &pRes);
        if (ecode != EC_SUCCESS) {
            syslog(LOG_ERR, "DB Query Error!");
            exit(EXIT_FAILURE);
        }
        syslog(LOG_DEBUG, "+----------+----------+--------+-------+");
        syslog(LOG_DEBUG, "| SiteCode | SiteName | SiteID | Basin |");
        syslog(LOG_DEBUG, "+----------+----------+--------+-------+");
        try {
            while ((sqlrow = db.db_fetch_row(pRes)) != NULL) {
                syslog(LOG_DEBUG, "|%9s |%9s |%7s |%6s |", sqlrow[0], sqlrow[1], sqlrow[2], sqlrow[3]);
                addr = sqlrow[2];
            }
        } catch (exception& e) {
            syslog(LOG_ERR, "DB Fetch Error!");
            cout << e.what() << endl;
        }
        syslog(LOG_DEBUG, "+----------+----------+--------+-------+");
        return addr;
    }
}
