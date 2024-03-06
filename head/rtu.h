#pragma once

#include "client.h"

namespace core {

    class RTUclient : public Client {
    private:
        SiteCode scode;
        int m_heartbeat_limits;
        
    public:
        RTUclient(ServerSocket& sock, int heartbeat_limits = 5);
        ~RTUclient();

        /**
         * @return true if SiteCode is available, false otherwise
        */
        bool isSiteCodeAvailable();

        /**
         * Converting Message Type to DATA type
        */
        virtual int reqMessage(DATA* buf, DATA cmd);
        virtual void run();
    };
}

