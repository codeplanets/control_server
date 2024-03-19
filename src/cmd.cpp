#include <unistd.h>
#include <signal.h>

#include "cmd.h"
#include "packetizer.h"
#include "socketexception.h"

void cmd_timeout_handler(int sig) {
    if (sig == SIGALRM) {
        syslog(LOG_WARNING, "Timeout %d seconds", CMD_WAITING_SEC);
    }
    exit(EXIT_FAILURE);
}

namespace core {
    CMDclient::CMDclient(ServerSocket& sock)
    : Client(sock) {
        
    }

    CMDclient::~CMDclient() {
        mq.close();
        updateStatus(false);
    }

    void CMDclient::init(ClientInitReq &msg, u_short addr) {
        this->serverAddr = msg.fromAddr;
        this->cmdAddr.setAddr(addr, CLIENT_ADDRESS);
        this->m_addr = addr;

        syslog(LOG_DEBUG, "CMDclient::init : Server-0x%02X Client-0x%02X", this->serverAddr.getAddr(), this->cmdAddr.getAddr());
    }

    void CMDclient::setSiteCode(char* scode) {
        this->scode.setSiteCode(scode);

        syslog(LOG_DEBUG, "CMDclient::setSiteCode : %s", this->scode.getSiteCode());
    }

    int CMDclient::reqMessage(DATA* buf, DATA cmd) {
        // CLIENT_INIT_RES, COMMAND_RTU, COMMAND_CLIENT_ACK
        if (cmd == CLIENT_INIT_RES) {
            ClientInitRes msg;
            syslog(LOG_DEBUG, "CMDclient::reqMessage : ClientInitRes size = %ld", sizeof(msg));

            // Packetizer
            msg.fromAddr = this->serverAddr;
            msg.toAddr = this->cmdAddr;
            msg.clientAddr = this->cmdAddr;
            msg.crc8.setCRC8(common::calcCRC((DATA*)&msg, sizeof(msg)));
            msg.print();

            memcpy(buf, (char*)&msg, sizeof(msg));
            common::print_hex(buf, sizeof(msg));
            return sizeof(msg);

        } else if (cmd == COMMAND_RTU) {
            CommandRtu msg;
            syslog(LOG_DEBUG, "CMDclient::reqMessage : CommandRtu size = %ld", sizeof(msg));
            
            // Packetizer
            msg.fromAddr = this->rtuAddr;
            msg.toAddr = this->cmdAddr;
            msg.siteCode = this->scode;
            msg.dcCommand = this->dcCommand;
            msg.acCommand = this->acCommand;
            msg.crc8.setCRC8(common::calcCRC((DATA*)&msg, sizeof(msg)));
            msg.print();

            memcpy(buf, (char*)&msg, sizeof(msg));
            common::print_hex(buf, sizeof(msg));
            return sizeof(msg);

        } else if (cmd == COMMAND_CLIENT_ACK) {
            CommandClientAck msg;
            syslog(LOG_DEBUG, "CMDclient::reqMessage : CommandClientAck size = %ld", sizeof(msg));

            // Packetizer
            msg.fromAddr = this->rtuAddr;
            msg.toAddr = this->cmdAddr;
            msg.siteCode = this->scode;
            msg.dcCommand = this->dcCommand;
            msg.acCommand = this->acCommand;
            msg.result = this->cmdResult;
            msg.crc8.setCRC8(common::calcCRC((DATA*)&msg, sizeof(msg)));
            msg.print();

            memcpy(buf, (char*)&msg, sizeof(msg));
            common::print_hex(buf, sizeof(msg));
            return sizeof(msg);

        } else if (cmd == SETUP_INFO_ACK) {
            SetupInfoAck msg;
            syslog(LOG_DEBUG, "CMDclient::reqMessage : SetupInfoAck size = %ld", sizeof(msg));
            
            // Packetizer
            msg.fromAddr = this->rtuAddr;
            msg.toAddr = this->cmdAddr;
            msg.action = this->action;
            msg.siteCode = this->scode;
            msg.result = this->actResult;
            msg.crc8.setCRC8(common::calcCRC((DATA*)&msg, sizeof(msg)));
            msg.print();

            memcpy(buf, (char*)&msg, sizeof(msg));
            common::print_hex(buf, sizeof(msg));
            return sizeof(msg);

        } else if (cmd == RTU_STATUS_RES) {
            RtuStatusResHead msgHead;
            RtuStatusResTail msgTail;
            syslog(LOG_DEBUG, "CMDclient::reqMessage : RtuStatusResHead size = %ld", sizeof(msgHead));

            // Packetizer
            msgHead.fromAddr = this->rtuAddr;
            // msg.toAddr = this->cmdAddr;
            msgHead.toAddr.setAddr(0x0FFF, CLIENT_ADDRESS);
            // set RtuStatus rtuStatus[];
            // msgHead.rtuStatus[i] = this->rtuStatus[i];
            msgHead.print();

            msgTail.crc8.setCRC8(common::calcCRC((DATA*)&msgHead, sizeof(msgHead)));

            memcpy(buf, (char*)&msgHead, sizeof(msgHead));
            memcpy(buf+sizeof(msgHead), (char*)&msgTail, sizeof(msgTail));
            common::print_hex(buf, sizeof(msgHead) + sizeof(msgTail));
            return sizeof(msgHead) + sizeof(msgTail);
        } else {
            syslog(LOG_WARNING, "Unknown message type. : 0x%X", cmd);
        }

        return 0;
    }

    void CMDclient::setTimeout() {
        struct sigaction action;
        action.sa_handler = cmd_timeout_handler;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        sigaction(SIGALRM, &action, NULL);
        alarm(CMD_WAITING_SEC);
    }

    void CMDclient::run() {
        
        DATA sock_buf[MAX_RAW_BUFF] = {0x00,};
        DATA mq_buf[MQ_MSGSIZE] = {0x00,};
        DATA sendbuf[MAX_RAW_BUFF] = {0x00, };

        int len = reqMessage(sendbuf, CLIENT_INIT_RES);
        newSock.send(sendbuf, len);

        createMessageQueue(CLIENT_MQ_NAME);
        updateStatus(true);

        u_short addr = this->cmdAddr.getAddr();
        pid_t pid = getpid();
        if (addr > 0) {
            read_mapper(CLIENT_DATA, mapper_list);
            int line = getTotalLine(CLIENT_DATA);
            mapper_list[line] = add_mapper(pid, addr);
            write_mapper(CLIENT_DATA, mapper_list);
            print_mapper(mapper_list);
        }

        while (true) {
            common::sleep(100);

            // TODO : Command RTU Message Queue
            try {
                errno = 0;
                int rcvByte = mq.recv(mq_buf, sizeof(mq_buf));
                if (rcvByte > 0) {
                    if (mq_buf[0] == STX) {
                        if (mq_buf[1] == COMMAND_RTU_ACK) {  // Client
                            syslog(LOG_INFO, "MQ >> SVR : Command RTU Ack.");
                            common::print_hex(mq_buf, rcvByte);

                            // updateDatabase(msg);
                            CommandRtuAck msg;
                            assert(rcvByte == sizeof(msg));
                            memcpy((void*)&msg, mq_buf, rcvByte);
                            if (common::checkCRC((DATA*)&msg, rcvByte, msg.crc8.getCRC8()) == false) {
                                syslog(LOG_WARNING, "CRC Check Failed. : 0x%02X != 0x%02X", common::calcCRC((DATA*)&msg, rcvByte), msg.crc8.getCRC8());
                            }
                            msg.print();

                            this->scode = msg.siteCode;
                            this->dcCommand = msg.dcCommand;
                            this->acCommand = msg.acCommand;
                            this->cmdResult = msg.result;

                            updateDatabase(true);
                            syslog(LOG_INFO, "SVR >> CMD : Command Client Ack.");
                            len = reqMessage(sock_buf, COMMAND_CLIENT_ACK);
                            newSock.send(sock_buf, len);
                        } else if (mq_buf[1] == RTU_STATUS_RES) {  // All Client
                            syslog(LOG_INFO, "MQ >> CMD : RTU Status Res.");
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
                        syslog(LOG_ERR, "CMDclient::run : %s", strerror(errno));
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
                    if (sock_buf[1] == COMMAND_CLIENT) {
                        syslog(LOG_INFO, "CMD >> SVR : Command Client.");
                        alarm(CMD_WAITING_SEC);

                        // insertDatabase(msg);
                        CommandClient msg;
                        assert(rcvByte == sizeof(msg));
                        memcpy((void*)&msg, sock_buf, rcvByte);
                        if (common::checkCRC((DATA*)&msg, rcvByte, msg.crc8.getCRC8()) == false) {
                            syslog(LOG_WARNING, "CRC Check Failed. : 0x%02X != 0x%02X", common::calcCRC((DATA*)&msg, rcvByte), msg.crc8.getCRC8());
                        }
                        msg.print();

                        this->scode = msg.siteCode;
                        this->dcCommand = msg.dcCommand;
                        this->acCommand = msg.acCommand;
                        
                        string addr = find_rtu_addr(this->scode);
                        if (addr != NOT_FOUND) {
                            this->rtuAddr.setAddr(addr, RTU_ADDRESS);
                        } else {
                            syslog(LOG_WARNING, "Unknown Site Code from client. : %s", this->scode.getSiteCode());
                            this->rtuAddr.setAddr(0x0000);
                        }
                        
                        // TODO : 해당 Address의 RTU MQ에 send 해야함.
                        syslog(LOG_INFO, "SVR >> MQ : Command RTU.");
                        sendByte = reqMessage(mq_buf, COMMAND_RTU);
                        Mq rtu_mq;
                        pid_t pid = 0;
                        std::vector<pid_t> pids;
                        // data에서 pid를 read.
                        read_mapper(RTU_DATA, mapper_list);
                        int line = getTotalLine(RTU_DATA);
                        search_mapper(mapper_list, pids, line, this->rtuAddr.getAddr());
                        cout << "RTU PIDs : ";
                        for (auto it = pids.begin(); it!= pids.end(); it++) {
                            cout << *it << " ";
                            pid = *it;
                            if (pid != 0) {
                                rtu_mq.open(RTU_MQ_NAME, pid);
                                rtu_mq.send(mq_buf, sendByte);
                                rtu_mq.close();
                            }
                        }
                        cout << endl;

                    } else if (sock_buf[1] == SETUP_INFO) {
                        syslog(LOG_INFO, "CMD >> SVR : Setup Info.");
                        alarm(CMD_WAITING_SEC);
                        
                        SetupInfo msg;
                        assert(rcvByte == sizeof(msg));
                        memcpy((void*)&msg, sock_buf, rcvByte);
                        if (common::checkCRC((DATA*)&msg, rcvByte, msg.crc8.getCRC8()) == false) {
                            syslog(LOG_WARNING, "CRC Check Failed. : 0x%02X != 0x%02X", common::calcCRC((DATA*)&msg, rcvByte), msg.crc8.getCRC8());
                        }
                        msg.print();
                        
                        // 값들을 저장
                        DATA result = ACTION_RESULT_OK;
                        bool shutdown = false;
                        this->serverAddr = msg.fromAddr;    // * 확인 *
                        this->rtuAddr = msg.toAddr;         // * 확인 *
                        this->action = msg.action;
                        this->scode = msg.siteCode;

                        // Action I, U, D
                        char action = this->action.getAction();
                        if (action != 'Q') {
                            syslog(LOG_INFO, "SVR >> DB : Setup Info Action %c!", this->action.getAction());
                            if (updateDatabase(true)) {
                                result = ACTION_RESULT_OK;
                            } else {
                                result = ACTION_RESULT_FAIL;
                            }
                        } else {
                            syslog(LOG_INFO, "Setup Info Action Q!");
                            shutdown = true;
                            result = ACTION_RESULT_OK;
                        }
                        
                        this->actResult.setResult(result);
                        
                        sendByte = reqMessage(sendbuf, SETUP_INFO_ACK);
                        newSock.send(sendbuf, sendByte);

                        if (shutdown) {
                            syslog(LOG_INFO, "Server Shutdown.");
                            break;
                            // TODO: kill process
                        }

                    } else if (sock_buf[1] == RTU_STATUS_REQ) {
                        syslog(LOG_INFO, "CMD >> SVR : RTU Status Req.");
                        alarm(CMD_WAITING_SEC);

                        RtuStatusReq msg;
                        assert(rcvByte == sizeof(msg));
                        memcpy((void*)&msg, sock_buf, rcvByte);
                        if (common::checkCRC((DATA*)&msg, rcvByte, msg.crc8.getCRC8()) == false) {
                            syslog(LOG_WARNING, "CRC Check Failed. : 0x%02X != 0x%02X", common::calcCRC((DATA*)&msg, rcvByte), msg.crc8.getCRC8());
                        }
                        msg.print();

                        this->serverAddr = msg.fromAddr;    // * 확인 *
                        this->cmdAddr = msg.toAddr;         // * 확인 *

                        // TODO : 모든 Address의 Client MQ에 send 해야함.
                        sendByte = reqMessage(sendbuf, RTU_STATUS_RES);
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
                                if (cmd_mq.open(CLIENT_MQ_NAME, pid)) {
                                    syslog(LOG_INFO, "SVR >> MQ : RTU Status Res. : %d", map->pid);
                                    cmd_mq.send(sendbuf, sendByte);
                                    cmd_mq.close();
                                } else {
                                    syslog(LOG_WARNING, "Failed to open Client MQ. : %d", pid);                                    
                                }
                            }
                        }
                        
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
    bool CMDclient::isSiteCodeAvailable() {
        string siteCode = find_rtu_addr(this->scode);
        if ( siteCode == NOT_FOUND) {
            return false;
        }
        syslog(LOG_DEBUG, "Site Code is available. : %s", siteCode.c_str());
        return true;
    }

    core::common::MAPPER CMDclient::add_mapper(int pid, u_short addr) {
        core::common::MAPPER mapper;
        mapper.pid = pid;
        mapper.addr = addr;
        return mapper;
    }

    void CMDclient::print_mapper(core::common::MAPPER* mapper) {
        for (int i = 0; i < MAX_POOL; i++) {
            if (mapper[i].pid != 0) {
                syslog(LOG_DEBUG, "Mapper : %d 0x%02X", mapper[i].pid, mapper[i].addr);
            }
        }
    }

    void CMDclient::search_mapper(core::common::MAPPER* mapper, pid_t &pid, int idx, u_short addr) {
        core::common::MAPPER* map;
        for (map = mapper; map < mapper + idx; map++) {
            if (map->addr == addr) {
                syslog(LOG_DEBUG, "Found Mapper : %d 0x%02X", map->pid, map->addr);

                pid = map->pid;
            }
        }
    }

    void CMDclient::search_mapper(core::common::MAPPER* mapper, std::vector<pid_t> &pids, int idx, u_short addr) {
        core::common::MAPPER* map;
        for (map = mapper; map < mapper + idx; map++) {
            if (map->addr == addr) {
                syslog(LOG_DEBUG, "Found Mapper : %d 0x%02X", map->pid, map->addr);

                pids.push_back(map->pid);
            }
        }
    }

    bool CMDclient::delete_mapper(core::common::MAPPER* mapper, int idx, int pid) {
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
    void CMDclient::write_mapper(std::string filename, core::common::MAPPER* mapper) {
        FILE * f = fopen(filename.c_str(), "a");
        for (int i = 0; i < MAX_POOL; i++) {
            if (mapper[i].pid != 0) {
                fprintf(f, "%d %hd\n", mapper[i].pid, mapper[i].addr);
            }
        }
        fclose(f);
    }

    void CMDclient::read_mapper(std::string filename, core::common::MAPPER* mapper) {
        FILE * f = fopen(filename.c_str(), "r");
        for (int i = 0; i < MAX_POOL; i++) {
            fscanf(f, "%d %hd", &mapper[i].pid, &mapper[i].addr);
        }
        fclose(f);
    }

    int CMDclient::getTotalLine(string name) {
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

    std::string CMDclient::find_rtu_addr(SiteCode scode) {
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
    bool CMDclient::insert_cmd_log() {
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
        return false;
    }

    bool CMDclient::update_cmd_log() {
        return false;
    }

}
