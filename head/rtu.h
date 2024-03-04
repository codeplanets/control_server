#pragma once

#include "message.h"
#include "mq.h"
#include "serversocket.h"

using namespace core::formatter;
using namespace core::server;
using namespace core::system;

namespace core {

    class RTUclient {
    private:
        SiteCode scode;
        int m_heartbeat_limits;
        
    protected:
        ServerSocket newSock;
        Address rtuaddr;
        Address clientaddr;
        Mq mq;
        bool m_isCreatedMq;
    
    public:
        RTUclient();
        RTUclient(ServerSocket& sock, int heartbeat_limits = 5);
        virtual ~RTUclient();

        void init(ServerSocket& sock);

        /**
         * @return true if SiteCode is available, false otherwise
        */
        bool isSiteCodeAvailable();

        bool createMessageQueue();

        bool updateStatus(bool status);
        bool insertDatabase(bool status);
        bool updateDatabase(bool status);

        /**
         * Converting Message Type to DATA type
        */
        const DATA* reqMessage(DATA cmd);

        void run();
    };
}

