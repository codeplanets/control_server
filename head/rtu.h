#pragma once

#include "client.h"

void rtu_timeout_handler(int sig);
namespace core {

    class RTUclient : public Client {
    public:
        RTUclient(ServerSocket& sock);
        ~RTUclient();

        void init(InitReq &msg);
        void setTimeout();
        
        /**
         * @return true if SiteCode is available, false otherwise
        */
        bool isSiteCodeAvailable();

        /**
         * Converting Message Type to DATA type
        */
        virtual int reqMessage(DATA* buf, DATA cmd);
        virtual void run();
    
    protected:
        std::string find_rtu_addr(SiteCode scode);
    };
}

