#include <unistd.h>
#include <signal.h>

#include "cmd.h"
#include "packetizer.h"
#include "socketexception.h"

void cmd_timeout_handler(int sig) {
    if (sig == SIGALRM) {
        syslog(LOG_WARNING, "Timeout %d seconds", waiting_sec * 5);
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

        cout << "CMDclient::init : " << this->serverAddr.getAddr() << " " << this->cmdAddr.getAddr() << endl;
    }

    void CMDclient::setSiteCode(char* scode) {
        this->scode.setSiteCode(scode);

        cout << "CMDclient::setSiteCode : " << this->scode.getSiteCode() << endl;
    }

    int CMDclient::reqMessage(DATA* buf, DATA cmd) {
        // CLIENT_INIT_RES, COMMAND_RTU, COMMAND_CLIENT_ACK
        if (cmd == CLIENT_INIT_RES) {
            ClientInitRes msg;
            cout << "ClientInitRes size = " << sizeof(msg) << endl;

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
            cout << "CommandRtu size = " << sizeof(msg) << endl;
            
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
            cout << "CommandClientAck size = " << sizeof(msg) << endl;

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
            cout << "SetupInfoAck size = " << sizeof(msg) << endl;
            
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
            RtuStatusRes msg;
            cout << "RtuStatusRes size = " << sizeof(msg) << endl;
            memcpy(buf, (char*)&msg, sizeof(msg));
            common::print_hex(buf, sizeof(msg));
            return sizeof(msg);
        }

        return 0;
    }

    void CMDclient::setTimeout() {
        struct sigaction action;
        action.sa_handler = cmd_timeout_handler;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        sigaction(SIGALRM, &action, NULL);
        alarm(waiting_sec * 5);
    }

    void CMDclient::run() {
        DATA sendbuf[MAX_RAW_BUFF] = {0x00, };
        int len = reqMessage(sendbuf, CLIENT_INIT_RES);
        
        newSock.send(sendbuf, len);

        createMessageQueue(client_mq_name);
        updateStatus(true);

        u_short addr = this->cmdAddr.getAddr();
        pid_t pid = getpid();
        if (addr > 0) {
            read_mapper(client_data, mapper_list);
            cout << getTotalLine(client_data) << endl;
            mapper_list[getTotalLine(client_data)] = add_mapper(pid, addr);
            write_mapper(client_data, mapper_list);
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
                    if (sock_buf[1] == COMMAND_CLIENT) {
                        syslog(LOG_DEBUG, "Command Client.");
                        alarm(waiting_sec);

                        // insertDatabase(msg);
                        CommandClient msg;
                        memcpy((void*)&msg, sock_buf, len);
                        if (common::checkCRC((DATA*)&msg, sizeof(msg), msg.crc8.getCRC8()) == false) {
                            syslog(LOG_WARNING, "CRC Check Failed. : 0x%02X != 0x%02X", common::calcCRC((DATA*)&msg, sizeof(msg)), msg.crc8.getCRC8());
                        }
                        msg.print();

                        this->scode = msg.siteCode;
                        this->dcCommand = msg.dcCommand;
                        this->acCommand = msg.acCommand;
                        
                        string addr = find_rtu_addr(this->scode);
                        if (addr != not_found) {
                            this->rtuAddr.setAddr(addr, RTU_ADDRESS);
                        } else {
                            syslog(LOG_WARNING, "Unknown Site Code from client. : %s", this->scode.getSiteCode());
                            this->rtuAddr.setAddr(0x0000);
                        }
                        
                        // TODO : 해당 Address의 RTU MQ에 send 해야함.
                        len = reqMessage(mq_buf, COMMAND_RTU);
                        Mq rtu_mq;
                        pid_t pid = 0;
                        std::vector<pid_t> pids;
                        // data에서 pid를 read.
                        read_mapper(rtu_data, mapper_list);
                        cout << getTotalLine(rtu_data) << endl;
                        search_mapper(mapper_list, pids, getTotalLine(rtu_data), this->rtuAddr.getAddr());
                        for (auto it = pids.begin(); it!= pids.end(); it++) {
                            cout << *it << endl;
                            pid = *it;
                            rtu_mq.open(rtu_mq_name, pid);
                            cout << pid << " < " << rtu_mq.send(mq_buf, len) << endl;
                            rtu_mq.close();
                        }

                    } else if (sock_buf[1] == SETUP_INFO) {
                        syslog(LOG_DEBUG, "Setup Info.");
                        alarm(waiting_sec);
                        
                        DATA result = ACTION_RESULT_OK;;
                        // ret = updateDatabase();
                        // if (ret) result = ACTION_RESULT_OK
                        // else result = ACTION_RESULT_FAIL
                        
                        SetupInfo msg;
                        memcpy((void*)&msg, sock_buf, len);
                        if (common::checkCRC((DATA*)&msg, sizeof(msg), msg.crc8.getCRC8()) == false) {
                            syslog(LOG_WARNING, "CRC Check Failed. : 0x%02X != 0x%02X", common::calcCRC((DATA*)&msg, sizeof(msg)), msg.crc8.getCRC8());
                        }
                        msg.print();
                        
                        this->serverAddr = msg.fromAddr;
                        this->rtuAddr = msg.toAddr;
                        this->action = msg.action;
                        this->scode = msg.siteCode;
                        this->actResult.setResult(result);
                        
                        len = reqMessage(sendbuf, SETUP_INFO_ACK);
                        newSock.send(sock_buf, len);

                    } else if (sock_buf[1] == RTU_STATUS_REQ) {
                        syslog(LOG_DEBUG, "RTU Status Req.");
                        // TODO : handle RTU Status
                        len = reqMessage(sendbuf, RTU_STATUS_RES);
                        newSock.send(sock_buf, len);

                    } else {
                        syslog(LOG_WARNING, "Unknown message type from socket.");
                    }
                } else {
                    syslog(LOG_WARNING, "Error Start of Text from socket.");
                }
            } catch (SocketException& se) {
                syslog(LOG_CRIT, "[Error : %s:%d] Exception was caught : [%d] %s",__FILE__, __LINE__, se.code(), se.description().c_str());
            }

            // TODO : Command RTU Message Queue
            try {
                errno = 0;
                if (mq.recv(mq_buf, sizeof(mq_buf))) {
                    if (mq_buf[0] == STX) {
                        if (mq_buf[1] == COMMAND_RTU_ACK) {  // Client
                            cout << "Command RTU Ack." << endl;

                            updateDatabase(true);
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
    bool CMDclient::isSiteCodeAvailable() {
        string siteCode = find_rtu_addr(this->scode);
        if ( siteCode == not_found) {
            return false;
        }
        cout << "contains this : " << siteCode << endl;
        return true;
    }

    core::common::MAPPER CMDclient::add_mapper(int pid, u_short addr) {
        core::common::MAPPER mapper;
        mapper.pid = pid;
        mapper.addr = addr;
        return mapper;
    }

    void CMDclient::print_mapper(core::common::MAPPER* mapper) {
        for (int i = 0; i < max_pool; i++) {
            if (mapper[i].pid != 0) {
                cout << mapper[i].pid << " " << mapper[i].addr << endl;
            }
        }
    }

    void CMDclient::search_mapper(core::common::MAPPER* mapper, pid_t &pid, int idx, u_short addr) {
        core::common::MAPPER* map;
        for (map = mapper; map < mapper + idx; map++) {
            if (map->addr == addr) {
                cout << map->pid << " " << map->addr << endl;
                pid = map->pid;
                break;
            }
        }
    }

    void CMDclient::search_mapper(core::common::MAPPER* mapper, std::vector<pid_t> &pids, int idx, u_short addr) {
        core::common::MAPPER* map;
        for (map = mapper; map < mapper + idx; map++) {
            if (map->addr == addr) {
                cout << map->pid << " " << map->addr << endl;
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
        for (int i = 0; i < max_pool; i++) {
            if (mapper[i].pid != 0) {
                fprintf(f, "%d %hd\n", mapper[i].pid, mapper[i].addr);
            }
        }
        fclose(f);
    }

    void CMDclient::read_mapper(std::string filename, core::common::MAPPER* mapper) {
        FILE * f = fopen(filename.c_str(), "r");
        for (int i = 0; i < max_pool; i++) {
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
