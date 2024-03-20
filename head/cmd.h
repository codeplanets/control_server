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
        Action action;
        Result actResult;

        CmdLog cmdLog;

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
    
        core::common::MAPPER add_mapper(int pid, u_short addr);
        void print_mapper(core::common::MAPPER* mapper);
        void search_mapper(core::common::MAPPER* mapper, pid_t &pid, int idx, u_short addr);
        void search_mapper(core::common::MAPPER* mapper, std::vector<pid_t> &pids, int idx, u_short addr);
        bool delete_mapper(core::common::MAPPER* mapper, int idx, int pid);
        void write_mapper(std::string filename, core::common::MAPPER* mapper);
        void read_mapper(std::string filename, core::common::MAPPER* mapper);
        int getTotalLine(std::string name);

        bool setup_init_value(CmdLog &log);
        bool setup_ack_value(CmdLog &log, std::string result, int code);
        bool insert_cmd_log(CmdLog &log);
        bool update_cmd_log(CmdLog &log);

    protected:
        std::string find_rtu_addr(SiteCode scode);
        
    private:
        core::common::MAPPER mapper_list[MAX_POOL] = {0, };
        
    };
}

