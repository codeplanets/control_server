#include <unistd.h>
#include <signal.h>

#include "cmd.h"
#include "packetizer.h"
#include "socketexception.h"

void cmd_timeout_handler(int sig) {
    if (sig == SIGALRM) {
        syslog(LOG_WARNING, "Timeout %d seconds", waiting_sec);
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

    void CMDclient::init(ClientInitReq &msg, std::string addr) {
        this->serverAddr = msg.fromAddr;
        this->cmdAddr.setAddr(addr, CLIENT_ADDRESS);

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
            msg.crc8.setCRC8(common::calcCRC((DATA*)&msg, sizeof(msg) - 2));
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
            msg.crc8.setCRC8(common::calcCRC((DATA*)&msg, sizeof(msg) - 2));
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
            msg.crc8.setCRC8(common::calcCRC((DATA*)&msg, sizeof(msg) - 2));
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
            msg.crc8.setCRC8(common::calcCRC((DATA*)&msg, sizeof(msg) - 2));
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
        alarm(waiting_sec);
    }

    void CMDclient::run() {
        DATA sendbuf[MAX_RAW_BUFF] = {0x00, };
        int len = reqMessage(sendbuf, CLIENT_INIT_RES);
        
        newSock.send(sendbuf, len);

        createMessageQueue(client_mq_name);
        updateStatus(true);

        DATA sock_buf[MAX_RAW_BUFF] = {0x00,};
        DATA mq_buf[MAX_RAW_BUFF] = {0x00,};

        while (true) {
            common::sleep(100);

            try {
                errno = 0;
                len = newSock.recv(sock_buf, MAX_RAW_BUFF);
                if (len < 0) {
                    if (errno == EAGAIN) {
                        common::sleep(1000);
                        continue;
                    } else {
                        cout << errno << ":" << strerror(errno) << endl;
                        break;
                    }
                } else if (len == 0) {
                    common::sleep(1000);
                    cout << ".";
                    continue;
                }

                if (sock_buf[0] == STX) {
                    if (sock_buf[1] == COMMAND_CLIENT) {
                        syslog(LOG_DEBUG, "Command Client.");

                        // insertDatabase(msg);
                        CommandClient msg;
                        memcpy((void*)&msg, sock_buf, len);
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

                        // TODO : 해당 SiteCode의 RTU MQ에 send 해야함.
                        len = reqMessage(mq_buf, COMMAND_RTU);
                        cout << mq.send(mq_buf, len) << endl;

                    } else if (sock_buf[1] == SETUP_INFO) {
                        syslog(LOG_DEBUG, "Setup Info.");
                        
                        DATA result = ACTION_RESULT_OK;;
                        // ret = updateDatabase();
                        // if (ret) result = ACTION_RESULT_OK
                        // else result = ACTION_RESULT_FAIL
                        
                        SetupInfo msg;
                        memcpy((void*)&msg, sock_buf, len);
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
            // cout << mq.recv(mq_buf, sizeof(mq_buf)) << endl;
            // if (mq_buf[0] == STX) {
            //     if (mq_buf[1] == COMMAND_RTU_ACK) {  // RTU
            //         cout << "[" << getpid() << "] : " << "Command RTU Ack." << endl;
            //         updateDatabase(true);
            //     } else {
            //         cout << "[" << getpid() << "] : " << "Unknown message type from mq." << hex << mq_buf[1] << endl;
            //     }
            // }else {
            //     cout << "[" << getpid() << "] : " << "Error Start of Text from mq." << hex << mq_buf[0] << endl;
            // }
            // sleep(1);
        }
    }
    
    /**
     * @return true if SiteCode is available, false otherwise
    */
    bool CMDclient::isSiteCodeAvailable() {
        // TODO: check if site code is available
        setSiteMap(this->sitesMap);
        print_map(this->sitesMap);
        
        char* siteCode = this->scode.getSiteCode();
        if (this->sitesMap.find(siteCode) != this->sitesMap.end()) {
            cout << "contains this : " << siteCode << endl;
            delete siteCode;
            return true;
        }

        delete siteCode;
        return true;
    }

    std::string CMDclient::find_rtu_addr(SiteCode scode) {
        char* siteCode = scode.getSiteCode();
        auto addr = this->sitesMap.find(siteCode);

        if (addr != this->sitesMap.end()) {
            cout << "SiteCode = " << addr->first << endl;
            cout << "RTU Addr = " << addr->second << endl;
            return addr->second;
        } else {
            return not_found;
        }
    }

    void CMDclient::print_map(std::map<std::string, std::string>& m) {
        for (std::map<std::string, std::string>::iterator itr = m.begin(); itr != m.end(); ++itr) {
            std::cout << itr->first << " " << itr->second << std::endl;
        }
    }

    void CMDclient::setSiteMap(std::map<std::string, std::string> &sc_map) {
        Database db;
        ECODE ecode = db.db_init("localhost", 3306, "rcontrol", "rcontrol2024", "RControl");
        if (ecode!= EC_SUCCESS) {
            syslog(LOG_ERR, "DB Connection Error!");
            exit(EXIT_FAILURE);
        }

        // Query Data
        MYSQL_ROW sqlrow;
        MYSQL_RES* pRes;
        ecode = db.db_query("select * from RSite", &pRes);
        if (ecode!= EC_SUCCESS) {
            syslog(LOG_ERR, "DB Query Error!");
            exit(EXIT_FAILURE);
        }

        syslog(LOG_DEBUG, "+----------+----------+--------+-------+");
        syslog(LOG_DEBUG, "| SiteCode | SiteName | SiteID | Basin |");
        syslog(LOG_DEBUG, "+----------+----------+--------+-------+");
        while ((sqlrow = db.db_fetch_row(pRes)) != NULL) {
            syslog(LOG_DEBUG, "|%9s |%9s |%7s |%6s |", sqlrow[0], sqlrow[1], sqlrow[2], sqlrow[3]);
            sc_map[sqlrow[0]] = sqlrow[2];
        }
        syslog(LOG_DEBUG, "+----------+----------+--------+-------+");
        print_map(sc_map);
        cout << sc_map.find("2000004")->second << endl;   // 14
    }

}
