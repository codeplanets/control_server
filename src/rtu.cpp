#include <unistd.h>

#include "rtu.h"
#include "packetizer.h"
#include "socketexception.h"

namespace core {
    RTUclient::RTUclient()
    : newSock(0)
    , m_isCreatedMq(false)
    , m_heartbeat_limits(5) {
        
    }
    
    RTUclient::RTUclient(ServerSocket& sock, int heartbeat_limits)
    : newSock(sock)
    , m_isCreatedMq(false)
    , m_heartbeat_limits(heartbeat_limits) {
        
    }

    RTUclient::~RTUclient() {
        updateStatus(false);
    }

    void RTUclient::init(ServerSocket& sock) {
        newSock = sock;
    }

    /**
     * @return true if SiteCode is available, false otherwise
    */
    bool RTUclient::isSiteCodeAvailable() {
        // TODO: check if site code is available
        return true;
    }

    bool RTUclient::createMessageQueue() {
        bool isCreated = mq.open(getpid());
        m_isCreatedMq = isCreated;

        // message queue 송수신 확인
        // const char* msg = "Message Queue Test OK!";
        // size_t msg_len = sizeof(msg);
        // cout << mq.send((DATA*)msg, msg_len) << endl;
        // DATA buf[MAX_RAW_BUFF] = {0x00,};
        // cout << mq.recv(buf, sizeof(buf)) << endl;

        return isCreated;
    }
    
    bool RTUclient::updateStatus(bool status) { return true; }
    bool RTUclient::insertDatabase(bool status) { return true; }
    bool RTUclient::updateDatabase(bool status) { return true; }

    int RTUclient::reqMessage(DATA* buf, DATA cmd) {
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

    void RTUclient::run() {
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

        int heartbeat_cnt = 0;

        while (true) {
            if (heartbeat_cnt > m_heartbeat_limits) {
                syslog(LOG_WARNING, "Timeout %d seconds", m_heartbeat_limits);
                break;
            } else {
                heartbeat_cnt++;
            }

            common::sleep(1000);

            try {
                int len = newSock.recv(sock_buf, MAX_RAW_BUFF);
                if (len <= 0) {
                    continue;
                }

                if (sock_buf[0] == STX) {
                    if (sock_buf[1] == HEART_BEAT) {  // RTU
                        syslog(LOG_DEBUG, "Heartbeat.");
                        heartbeat_cnt = 0;

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
