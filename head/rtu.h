#pragma once

#include "client.h"
#include <set>

void rtu_timeout_handler(int sig);
namespace core {

    class RTUclient : public Client {
    public:
        RTUclient(ServerSocket& sock);
        ~RTUclient();

        // void init(InitReq &msg);
        void setTimeout();
        
        /**
         * Converting Message Type to DATA type
        */
        virtual int reqMessage(DATA* buf, DATA cmd);
        virtual void run();

        void setStatus(string code, DATA status);
    
    protected:
        int shm_fd;
        void* shm_ptr;
    private:
        core::common::MAPPER cmd_mapper_list[MAX_POOL] = {0, };
        // core::common::MAPPER mapper_list[MAX_POOL] = {0, };
    };
}

