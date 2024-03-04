#include <unistd.h>

#include "rtu.h"
#include "packetizer.h"

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
        const char* msg = "Message Queue Test OK!";
        size_t msg_len = sizeof(msg);
        cout << mq.send(msg, msg_len) << endl;
        DATA buf[MAX_RAW_BUFF] = {0x00,};
        cout << mq.recv(buf, sizeof(buf)) << endl;

        return isCreated;
    }
    
    bool RTUclient::updateStatus(bool status) { return true; }
    bool RTUclient::insertDatabase(bool status) { return true; }
    bool RTUclient::updateDatabase(bool status) { return true; }

    const DATA* RTUclient::reqMessage(DATA cmd) {
        // INIT_RES, HEART_BEAT_ACK, COMMAND_RTU
        if (cmd == INIT_RES) {
            // core::server::Packetizer::getMessage(cmd);
            InitRes msg;
            msg.stx = STX;
            msg.cmd = cmd;
            // msg.fromAddr.setBigEndian("", 2);
            // msg.toAddr.setBigEndian("", 2);
            msg.length = 10;
            // msg.siteCode.setSiteCode("");
            // msg.rtuAddr.setBigEndian("", 2);
            msg.crc8;
            msg.etx = ETX;
            
            return NULL;
        } else if (cmd == HEART_BEAT_ACK) {
            // core::server::Packetizer::getMessage(cmd);
            return NULL;
        } else if (cmd == COMMAND_RTU) {
            // core::server::Packetizer::getMessage(cmd);
            return NULL;
        }
        return NULL;
    }

    void RTUclient::run() {
        if (isSiteCodeAvailable() == false) {
            newSock << reqMessage(INIT_RES);
            sleep(5);
            return;
        }

        newSock << reqMessage(INIT_RES);
        createMessageQueue();
        updateStatus(true);

        DATA sock_buf[MAX_RAW_BUFF] = {0x00,};
        // DATA mq_buf[MAX_RAW_BUFF] = {0x00,};

        int heartbeat_cnt = 0;

        while (true) {
            if (heartbeat_cnt > m_heartbeat_limits) {
                cout << "[" << getpid() << "] : " << "Timeout." << endl;
                break;
            } else {
                heartbeat_cnt++;
            }

            sleep(1);

            newSock >> sock_buf;

            if (sock_buf[0] == STX) {
                if (sock_buf[1] == HEART_BEAT) {  // RTU
                    cout << "[" << getpid() << "] : " << "Heartbeat." << endl;
                    heartbeat_cnt = 0;
                    sock_buf[1] = HEART_BEAT_ACK;
                    newSock << sock_buf;
                } else if (sock_buf[1] == COMMAND_RTU_ACK) {  // Client
                    cout << "[" << getpid() << "] : " << "Command RTU Ack." << endl;

                    size_t msg_len = sizeof(sock_buf);
                    cout << mq.send(sock_buf, msg_len) << endl;

                    // updateDatabase();
                } else {
                    cout << "[" << getpid() << "] : " << "Unknown message type from socket." << hex << sock_buf[1] << endl;
                }
            } else {
                cout << "[" << getpid() << "] : " << "Error Start of Text from socket." << hex << sock_buf[0] << endl;
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
