#include <unistd.h>
#include <signal.h>

#include "rtu.h"
#include "packetizer.h"
#include "socketexception.h"

void rtu_timeout_handler(int sig) {
    if (sig == SIGALRM) {
        syslog(LOG_WARNING, "Timeout %d seconds", waiting_sec);
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
        cout << "RTUclient::init" << endl;

        this->rtuAddr = msg.fromAddr;
        this->serverAddr = msg.toAddr;
        this->scode = msg.siteCode;

        cout << this->rtuAddr.getAddr() << " " << this->serverAddr.getAddr() << " " << this->scode.getSiteCode() << endl;
    }

    int RTUclient::reqMessage(DATA* buf, DATA cmd) {
        // INIT_RES, HEART_BEAT_ACK, COMMAND_RTU
        if (cmd == INIT_RES) {
            InitRes msg;
            cout << "InitRes size = " << sizeof(msg) << endl;

            string addr = find_rtu_addr(this->scode);
            if (addr != not_found) {
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
            // msg.print();

            memcpy(buf, (char*)&msg, sizeof(msg));
            common::print_hex(buf, sizeof(msg));
            return sizeof(msg);

        } else if (cmd == HEART_BEAT_ACK) {
            HeartBeatAck msg;
            cout << "Heartbeat Ack size = " << sizeof(msg) << endl;
            
            msg.fromAddr = this->rtuAddr;
            msg.toAddr = this->serverAddr;

            msg.crc8.setCRC8(common::calcCRC((DATA*)&msg, sizeof(msg)));
            // msg.print();

            memcpy(buf, (char*)&msg, sizeof(msg));
            common::print_hex(buf, sizeof(msg));
            return sizeof(msg);

        } else if (cmd == COMMAND_RTU) {
            CommandRtu msg;
            cout << "Command RTU size = " << sizeof(msg) << endl;

            msg.crc8.setCRC8(common::calcCRC((DATA*)&msg, sizeof(msg)));

            memcpy(buf, (char*)&msg, sizeof(msg));
            common::print_hex(buf, sizeof(msg));
            return sizeof(msg);

        }

        return 0;
    }
    
    void RTUclient::setTimeout() {
        struct sigaction action;
        action.sa_handler = rtu_timeout_handler;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        sigaction(SIGALRM, &action, NULL);
        alarm(waiting_sec);
    }

    void RTUclient::run() {
        
        DATA sendbuf[MAX_RAW_BUFF] = {0x00, };
        int len = reqMessage(sendbuf, INIT_RES);

        if (isSiteCodeAvailable() == false) {
            // TODO : set rtu address 0x0000

            newSock.send(sendbuf, len);

            common::sleep(5000);
            return;
        }
        len = reqMessage(sendbuf, INIT_RES);
        newSock.send(sendbuf, len);

        createMessageQueue(rtu_mq_name);
        updateStatus(true);

        u_short addr = this->rtuAddr.getAddr();
        pid_t pid = getpid();
        if (addr > 0) {
            read_mapper(rtu_data, mapper_list);
            cout << getTotalLine(rtu_data) << endl;
            mapper_list[getTotalLine(rtu_data)] = add_mapper(pid, addr);
            write_mapper(rtu_data, mapper_list);
            print_mapper(mapper_list);
        }

        DATA sock_buf[MAX_RAW_BUFF] = {0x00,};
        DATA mq_buf[MAX_RAW_BUFF] = {0x00,};

        while (true) {
            common::sleep(100);

            try {
                errno = 0;
                len = newSock.recv(sock_buf, MAX_RAW_BUFF);
                if (len < 0) {
                    if (errno == EAGAIN) {
                        common::sleep(100);
                        continue;
                    } else {
                        cout << errno << ":" << strerror(errno) << endl;
                        break;
                    }
                } else if (len == 0) {
                    common::sleep(100);
                    cout << ".";
                    continue;
                }

                if (sock_buf[0] == STX) {
                    if (sock_buf[1] == INIT_REQ) {  // RTUs
                        syslog(LOG_DEBUG, "RTU Init Request");
                        alarm(waiting_sec);
                        InitReq msg;
                        memcpy((void*)&msg, sock_buf, sizeof(msg));
                        if (common::checkCRC((DATA*)&msg, sizeof(msg), msg.crc8.getCRC8()) == false) {
                            syslog(LOG_WARNING, "CRC Check Failed. : 0x%02X != 0x%02X", common::calcCRC((DATA*)&msg, sizeof(msg)), msg.crc8.getCRC8());
                        }
                        // msg.print();

                        if (isSiteCodeAvailable() == false) {
                            newSock.send(sendbuf, len);
                            common::sleep(5000);
                            return;
                        }
                        len = reqMessage(sendbuf, INIT_RES);
                        newSock.send(sendbuf, len);

                    } else if (sock_buf[1] == HEART_BEAT) {  // RTU
                        syslog(LOG_DEBUG, "Heartbeat.");
                        alarm(waiting_sec);
                        HeartBeat msg;
                        memcpy((void*)&msg, sock_buf, sizeof(msg));
                        if (common::checkCRC((DATA*)&msg, sizeof(msg), msg.crc8.getCRC8()) == false) {
                            syslog(LOG_WARNING, "CRC Check Failed. : 0x%02X != 0x%02X", common::calcCRC((DATA*)&msg, sizeof(msg)), msg.crc8.getCRC8());
                        }
                        // msg.print();
                        

                        len = reqMessage(sendbuf, HEART_BEAT_ACK);
                        newSock.send(sendbuf, len);

                    } else if (sock_buf[1] == COMMAND_RTU_ACK) {  // Client
                        syslog(LOG_DEBUG, "Command RTU Ack.");
                        CommandRtuAck msg;
                        memcpy((void*)&msg, sock_buf, sizeof(msg));
                        if (common::checkCRC((DATA*)&msg, sizeof(msg), msg.crc8.getCRC8()) == false) {
                            syslog(LOG_WARNING, "CRC Check Failed. : 0x%02X != 0x%02X", common::calcCRC((DATA*)&msg, sizeof(msg)), msg.crc8.getCRC8());
                        }
                        msg.print();

                        // TODO : Client MQ 에 데이터를 전송
                        Mq cmd_mq;
                        pid_t pid = 0;
                        std::vector<pid_t> pids;
                        // data에서 pid를 read.
                        read_mapper(client_data, mapper_list);
                        cout << getTotalLine(client_data) << endl;
                        search_mapper(mapper_list, pids, getTotalLine(client_data), msg.toAddr.getAddr());
                        for (auto it = pids.begin(); it!= pids.end(); it++) {
                            cout << *it << endl;
                            pid = *it;
                            cmd_mq.open(client_mq_name, pid);
                            cout << pid << " < " << cmd_mq.send(sock_buf, sizeof(msg)) << endl;
                            cmd_mq.close();
                        }
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

            // TODO : Command Client Message Queue
            try {
                errno = 0;
                if (mq.recv(mq_buf, sizeof(mq_buf))) {
                    if (mq_buf[0] == STX) {
                        if (mq_buf[1] == COMMAND_RTU) {  // Client
                            cout << "[" << getpid() << "] : " << "Command Client." << endl;
                            insertDatabase(true);
                            newSock.send(mq_buf, sizeof(mq_buf));
                        } else {
                            cout << "[" << getpid() << "] : " << "Unknown message type from mq." << hex << mq_buf[1] << endl;
                        }
                    }else {
                        cout << "[" << getpid() << "] : " << "Error Start of Text from mq." << hex << mq_buf[0] << endl;
                    }
                }
            // sleep(1);
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
        if ( siteCode == not_found) {
            return false;
        }
        cout << "contains this : " << siteCode << endl;
        return true;
    }

    core::common::MAPPER RTUclient::add_mapper(int pid, u_short addr) {
        core::common::MAPPER mapper;
        mapper.pid = pid;
        mapper.addr = addr;
        return mapper;
    }

    void RTUclient::print_mapper(core::common::MAPPER* mapper) {
        for (int i = 0; i < max_pool; i++) {
            if (mapper[i].pid != 0) {
                cout << mapper[i].pid << " " << mapper[i].addr << endl;
            }
        }
    }

    void RTUclient::search_mapper(core::common::MAPPER* mapper, pid_t &pid, int idx, u_short addr) {
        core::common::MAPPER* map;
        for (map = mapper; map < mapper + idx; map++) {
            if (map->addr == addr) {
                cout << map->pid << " " << map->addr << endl;
                pid = map->pid;
                break;
            }
        }
    }

    void RTUclient::search_mapper(core::common::MAPPER* mapper, std::vector<pid_t> &pids, int idx, u_short addr) {
        core::common::MAPPER* map;
        for (map = mapper; map < mapper + idx; map++) {
            if (map->addr == addr) {
                cout << map->pid << " " << map->addr << endl;
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
        for (int i = 0; i < max_pool; i++) {
            if (mapper[i].pid != 0) {
                fprintf(f, "%d %hd\n", mapper[i].pid, mapper[i].addr);
            }
        }
        fclose(f);
    }

    void RTUclient::read_mapper(std::string filename, core::common::MAPPER* mapper) {
        FILE * f = fopen(filename.c_str(), "r");
        core::common::MAPPER compare;
        for (int i = 0; i < max_pool; i++) {
            // fscanf(f, "%d %hd", &mapper[i].pid, &mapper[i].addr);
            fscanf(f, "%d %hd", &compare.pid, &compare.addr);
            
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
        string addr = not_found;
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
        cout << query << endl;
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
            cout << e.what() << endl;
        }
        syslog(LOG_DEBUG, "+----------+----------+--------+-------+");
        return addr;
    }
}
