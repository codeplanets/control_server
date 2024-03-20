#pragma once

#include "client.h"
#include "cmdlog.h"

void cmd_timeout_handler(int sig);
namespace core {
    
    class CMDclient : public Client {
    private:
        Command dcCommand;
        Command acCommand;
        Result cmdResult;

        CmdLog cmdLog;

    public:
        CMDclient(ServerSocket& sock);
        ~CMDclient();

        void init(ClientInitReq &msg, u_short addr);
        void setSiteCode(char* scode);
        void setTimeout();
        
        /**
         * Converting Message Type to DATA type
        */
        virtual int reqMessage(DATA* buf, DATA cmd);
        virtual void run();
    
        bool setup_init_value(CmdLog &log);
        bool setup_ack_value(CmdLog &log, std::string result, int code);
        bool insert_cmd_log(CmdLog &log);
        bool update_cmd_log(CmdLog &log);

    protected:
        
    private:
        // core::common::MAPPER mapper_list[MAX_POOL] = {0, };
        
    };
}

