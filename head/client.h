#pragma once

#include "message.h"
#include "serversocket.h"
#include "mq.h"
#include "db.h"

using namespace core::formatter;
using namespace core::server;
using namespace core::system;

namespace core {

    class Client {
    protected:
        ServerSocket newSock;
        Address rtuAddr;
        Address serverAddr;
        Address cmdAddr;
        SiteCode scode;
        Mq mq;
        u_short m_addr;
        bool m_isCreatedMq;
    
    public:
        Client(ServerSocket& sock);
        ~Client();

        u_short getCmdAddr() { return m_addr; }
        /**
         * @return true if SiteCode is available, false otherwise
        */
        bool createMessageQueue(std::string mq_name);

        /**
         * Converting Message Type to DATA type
        */
        virtual int reqMessage(DATA* buf, DATA cmd) = 0;
        virtual void run() = 0;

        virtual bool updateStatus(bool status);
        virtual bool insertDatabase(bool status);
        virtual bool updateDatabase(bool status);
    };
}

