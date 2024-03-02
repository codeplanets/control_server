#pragma once

#include "message.h"
#include "serversocket.h"

using namespace core::formatter;
using namespace core::server;

namespace core {

    class RTUclient {
    private:
        SiteCode scode;
        
    protected:
        ServerSocket newSock;
        Address rtuaddr;
        Address clientaddr;
    
    public:
        RTUclient();
        virtual ~RTUclient();

        void init(ServerSocket& sock);

        /**
         * @return true if SiteCode is available, false otherwise
        */
        bool isSiteCodeAvailable();
        void createMessageQueue();
        bool updateStatus(bool status);

        const DATA* reqMessage(DATA cmd);  // INIT_RES, 

        void run();
    };
}

