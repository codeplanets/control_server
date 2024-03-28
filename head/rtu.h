#pragma once

#include "client.h"
#include <set>

void rtu_timeout_handler(int sig);
namespace core {

    class RTUclient : public Client {
    protected:
        int shm_fd;
        void* shm_ptr;

    private:
        core::common::Mapper cmd_mapper_list[MAX_POOL];

        std::pair<pid_t, u_short> add_pair(pid_t pid, u_short addr);
        void print_pair(std::set<std::pair<pid_t, u_short>> s);
        bool search_pair(std::set<std::pair<pid_t, u_short>> s, u_short addr, pid_t &pid);
        bool search_pair(std::set<std::pair<pid_t, u_short>> s, pid_t pid, u_short &addr);
        bool search_pair(std::set<std::pair<pid_t, u_short>> s, u_short addr, std::vector<pid_t> &pids);
        bool delete_pair(std::set<std::pair<pid_t, u_short>> &s, int pid);
        void write_pair(std::string filename, std::set<std::pair<pid_t, u_short>> s);
        void read_pair(std::string filename, std::set<std::pair<pid_t, u_short>> &s);

    public:
        RTUclient(ServerSocket& sock);
        ~RTUclient();

        // void init(InitReq &msg);
        void setTimeout();
        
        virtual int reqMessage(DATA* buf, DATA cmd);
        virtual void run();

        void setStatus(u_short code, DATA status);
    };
}

