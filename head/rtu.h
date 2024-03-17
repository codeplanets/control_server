#pragma once

#include "client.h"
#include <set>

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
    
        core::common::MAPPER add_mapper(int pid, u_short addr);
        void print_mapper(core::common::MAPPER* mapper);
        void search_mapper(core::common::MAPPER* mapper, pid_t &pid, int idx, u_short addr);
        void search_mapper(core::common::MAPPER* mapper, std::vector<pid_t> &pids, int idx, u_short addr);
        bool delete_mapper(core::common::MAPPER* mapper, int idx, int pid);
        void write_mapper(std::string filename, core::common::MAPPER* mapper);
        void read_mapper(std::string filename, core::common::MAPPER* mapper);
        int getTotalLine(string name);

    protected:
        std::string find_rtu_addr(SiteCode scode);

    private:
        core::common::MAPPER mapper_list[max_pool] = {0, };
        
    };
}

