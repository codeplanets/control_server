#pragma once

#include "client.h"

namespace core {

    class CMDclient : public Client {
    private:
        SiteCode scode;
        
    public:
        CMDclient(ServerSocket& sock);
        ~CMDclient();

        /**
         * @return true if SiteCode is available, false otherwise
        */
        virtual bool createMessageQueue();

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

