#include <unistd.h>

#include "cmd.h"
#include "packetizer.h"
#include "socketexception.h"

namespace core {
    CMDclient::CMDclient(ServerSocket& sock)
    : Client(sock) {
        
    }

    CMDclient::~CMDclient() {
        mq.close();
        updateStatus(false);
    }

    bool CMDclient::createMessageQueue() {
        bool isCreated = mq.open(client_mq_name, getpid());
        m_isCreatedMq = isCreated;
        return isCreated;
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
        if (cmd == CLIENT_INIT_RES) {
            ClientInitRes msg;
            cout << sizeof(msg) << endl;
            memcpy(buf, (char*)&msg, sizeof(msg));
            common::print_hex(buf, sizeof(msg));
            return sizeof(msg);
        } else if (cmd == COMMAND_CLIENT_ACK) {
            CommandClientAck msg;
            cout << sizeof(msg) << endl;
            memcpy(buf, (char*)&msg, sizeof(msg));
            common::print_hex(buf, sizeof(msg));
            return sizeof(msg);
        } else if (cmd == SETUP_INFO_ACK) {
            SetupInfoAck msg;
            cout << sizeof(msg) << endl;
            memcpy(buf, (char*)&msg, sizeof(msg));
            common::print_hex(buf, sizeof(msg));
            return sizeof(msg);
        }

        return 0;
    }

    void CMDclient::run() {
        DATA sendbuf[MAX_RAW_BUFF] = {0x00, };
    
        int len = reqMessage(sendbuf, CLIENT_INIT_RES);
        cout << sizeof(sendbuf) << endl;

        newSock.send(sendbuf, len);

        createMessageQueue();
        updateStatus(true);

        DATA sock_buf[MAX_RAW_BUFF] = {0x00,};
        // DATA mq_buf[MAX_RAW_BUFF] = {0x00,};

        while (true) {
            common::sleep(1000);

            try {
                len = newSock.recv(sock_buf, MAX_RAW_BUFF);
                if (len <= 0) {
                    continue;
                }

                if (sock_buf[0] == STX) {
                    if (sock_buf[1] == COMMAND_CLIENT) {
                        syslog(LOG_DEBUG, "Command Client.");
                        // TODO : handle command client
                        cout << mq.send(sock_buf, sizeof(CommandClientAck)) << endl;

                    } else if (sock_buf[1] == SETUP_INFO) {
                        syslog(LOG_DEBUG, "Setup Info.");
                        // TODO : handle setup info
                        // updateDatabase();
                        len = reqMessage(sendbuf, SETUP_INFO_ACK);
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
}
