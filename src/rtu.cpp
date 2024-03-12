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
            msg.crc8.setCRC8(common::calcCRC((DATA*)&msg, sizeof(msg) - 2));
            msg.print();

            memcpy(buf, (char*)&msg, sizeof(msg));
            common::print_hex(buf, sizeof(msg));
            return sizeof(msg);

        } else if (cmd == HEART_BEAT_ACK) {
            HeartBeatAck msg;
            cout << "Heartbeat Ack size = " << sizeof(msg) << endl;
            
            msg.fromAddr = this->rtuAddr;
            msg.toAddr = this->serverAddr;

            msg.crc8.setCRC8(common::calcCRC((DATA*)&msg, sizeof(msg) - 2));
            msg.print();

            memcpy(buf, (char*)&msg, sizeof(msg));
            common::print_hex(buf, sizeof(msg));
            return sizeof(msg);

        } else if (cmd == COMMAND_RTU) {
            CommandRtu msg;
            cout << "Command RTU size = " << sizeof(msg) << endl;

            msg.crc8.setCRC8(common::calcCRC((DATA*)&msg, sizeof(msg) - 2));

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

        DATA sock_buf[MAX_RAW_BUFF] = {0x00,};
        // DATA mq_buf[MAX_RAW_BUFF] = {0x00,};

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
                    if (sock_buf[1] == HEART_BEAT) {  // RTU
                        syslog(LOG_DEBUG, "Heartbeat.");
                        alarm(waiting_sec);

                        len = reqMessage(sendbuf, HEART_BEAT_ACK);
                        newSock.send(sendbuf, len);

                    } else if (sock_buf[1] == COMMAND_RTU_ACK) {  // Client
                        syslog(LOG_DEBUG, "Command RTU Ack.");

                        size_t msg_len = sizeof(sock_buf);
                        cout << mq.send(sock_buf, msg_len) << endl;

                        // updateDatabase();
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
            // cout << mq.recv(mq_buf, sizeof(mq_buf)) << endl;
            // if (mq_buf[0] == STX) {
            //     if (mq_buf[1] == COMMAND_RTU) {  // Client
            //         cout << "[" << getpid() << "] : " << "Command Client." << endl;
            //         insertDatabase(true);
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
    bool RTUclient::isSiteCodeAvailable() {
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

    std::string RTUclient::find_rtu_addr(SiteCode scode) {
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

    void RTUclient::print_map(std::map<std::string, std::string>& m) {
        for (std::map<std::string, std::string>::iterator itr = m.begin(); itr != m.end(); ++itr) {
            std::cout << itr->first << " " << itr->second << std::endl;
        }
    }

    void RTUclient::setSiteMap(std::map<std::string, std::string> &sc_map) {
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
        try {
            while ((sqlrow = db.db_fetch_row(pRes)) != NULL) {
                syslog(LOG_DEBUG, "|%9s |%9s |%7s |%6s |", sqlrow[0], sqlrow[1], sqlrow[2], sqlrow[3]);
                sc_map[sqlrow[0]] = sqlrow[2];
            }
        } catch (exception& e) {
            cout << e.what() << endl;
        }
        
        syslog(LOG_DEBUG, "+----------+----------+--------+-------+");
        // print_map(sc_map);
        cout << sc_map.find("2000004")->second << endl;   // 14
    }

}
