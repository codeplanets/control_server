#pragma once

#include "message.h"
#include "serversocket.h"
#include "mq.h"

using namespace core::formatter;
using namespace core::server;
using namespace core::system;

namespace core {

    class Client {
    protected:
        ServerSocket newSock;
        Address rtuaddr;
        Address clientaddr;
        Mq mq;
        bool m_isCreatedMq;
    
    public:
        Client(ServerSocket& sock);
        ~Client();
        void init(ServerSocket& sock);

        /**
         * @return true if SiteCode is available, false otherwise
        */
        virtual bool createMessageQueue();

        virtual bool updateStatus(bool status);
        virtual bool insertDatabase(bool status);
        virtual bool updateDatabase(bool status);

        /**
         * Converting Message Type to DATA type
        */
        virtual int reqMessage(DATA* buf, DATA cmd) = 0;
        virtual void run() = 0;
    };
}

