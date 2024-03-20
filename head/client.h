#pragma once

#include "message.h"
#include "serversocket.h"
#include "mq.h"
#include "db.h"

using namespace core::formatter;
using namespace core::server;
using namespace core::system;

namespace core {

    class Client {
    protected:
        ServerSocket newSock;
        Address rtuAddr;
        Address serverAddr;
        Address cmdAddr;
        SiteCode scode;
        Mq mq;
        u_short m_addr;
        bool m_isCreatedMq;
    
        core::common::MAPPER mapper_list[MAX_POOL] = {0, };

    public:
        Client(ServerSocket& sock);
        ~Client();

        /**
         * @return true if SiteCode is available, false otherwise
        */
        bool isSiteCodeAvailable();

        u_short getCmdAddr() { return m_addr; }
        /**
         * @return true if SiteCode is available, false otherwise
        */
        bool createMessageQueue(std::string mq_name);

        /**
         * Converting Message Type to DATA type
        */
        virtual int reqMessage(DATA* buf, DATA cmd) = 0;
        virtual void run() = 0;

        std::string find_rtu_addr(SiteCode scode);

        core::common::MAPPER add_mapper(int pid, u_short addr);
        void print_mapper(core::common::MAPPER* mapper);
        void search_mapper(core::common::MAPPER* mapper, pid_t &pid, int idx, u_short addr);
        void search_mapper(core::common::MAPPER* mapper, std::vector<pid_t> &pids, int idx, u_short addr);
        bool delete_mapper(core::common::MAPPER* mapper, int idx, int pid);
        void write_mapper(std::string filename, core::common::MAPPER* mapper);
        void read_mapper(std::string filename, core::common::MAPPER* mapper);
        int getTotalLine(std::string name);

        virtual bool updateStatus(bool status);
        virtual bool updateDatabase(bool status);
    };
}

