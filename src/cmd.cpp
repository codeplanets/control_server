#include <unistd.h>

#include "cmd.h"
#include "packetizer.h"
#include "socketexception.h"

namespace core {
    CMDclient::CMDclient(ServerSocket& sock)
    : Client(sock) {
        
    }

    CMDclient::~CMDclient() {
        updateStatus(false);
    }

    /**
     * @return true if SiteCode is available, false otherwise
    */
    bool CMDclient::isSiteCodeAvailable() {
        // TODO: check if site code is available
        return true;
    }

    int CMDclient::reqMessage(DATA* buf, DATA cmd) {
        // INIT_RES, HEART_BEAT_ACK, COMMAND_RTU
        if (cmd == INIT_RES) {
            InitRes msg;
            cout << sizeof(msg) << endl;
            memcpy(buf, (char*)&msg, sizeof(msg));
            common::print_hex(buf, sizeof(msg));
            return sizeof(msg);
        } else if (cmd == HEART_BEAT_ACK) {
            HeartBeatAck msg;
            cout << sizeof(msg) << endl;
            memcpy(buf, (char*)&msg, sizeof(msg));
            common::print_hex(buf, sizeof(msg));
            return sizeof(msg);
        } else if (cmd == COMMAND_RTU) {
            CommandRtu msg;
            cout << sizeof(msg) << endl;
            memcpy(buf, (char*)&msg, sizeof(msg));
            common::print_hex(buf, sizeof(msg));
            return sizeof(msg);
        }

        return 0;
    }

    void CMDclient::run() {
        DATA sendbuf[MAX_RAW_BUFF] = {0x00, };
    
        int len = reqMessage(sendbuf, INIT_RES);
        cout << sizeof(sendbuf) << endl;

        newSock.send(sendbuf, len);

        if (isSiteCodeAvailable() == false) {
            common::sleep(5000);
            return;
        }

        createMessageQueue();
        updateStatus(true);

        DATA sock_buf[MAX_RAW_BUFF] = {0x00,};
        // DATA mq_buf[MAX_RAW_BUFF] = {0x00,};

        while (true) {
            common::sleep(1000);

            try {
                int len = newSock.recv(sock_buf, MAX_RAW_BUFF);
                if (len <= 0) {
                    continue;
                }

                if (sock_buf[0] == STX) {
                    if (sock_buf[1] == HEART_BEAT) {  // RTU
                        syslog(LOG_DEBUG, "Heartbeat.");

                        sock_buf[1] = HEART_BEAT_ACK;
                        newSock.send(sock_buf, sizeof(HeartBeatAck));

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
            //     if (mq_buf[1] == COMMAND_CLIENT) {  // Client
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
}
