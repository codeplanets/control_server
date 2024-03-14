#pragma once

#include "client.h"

void cmd_timeout_handler(int sig);
namespace core {

    class CMDclient : public Client {
    private:
        Command dcCommand;
        Command acCommand;
        CommandResult cmdResult;
        Action action;
        ActionResult actResult;

    public:
        CMDclient(ServerSocket& sock);
        ~CMDclient();

        void init(ClientInitReq &msg, u_short addr);
        void setSiteCode(char* scode);
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

