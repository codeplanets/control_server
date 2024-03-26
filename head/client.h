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
        Action action;
        Result actResult;
        Mq mq;
        u_short m_addr;
        bool m_isCreatedMq;
    
        core::common::Mapper mapper_list[MAX_POOL];

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

        /**
         * Runner
        */
        virtual void run() = 0;

        /**
         * @return Getting sitecode count from database
        */
        size_t getcount_site();

        /**
         * @return Getting sitdID from database to sitecode
        */
        std::string find_rtu_addr(SiteCode scode);

        /**
         * Control mapper class
        */
        core::common::Mapper add_mapper(int pid, u_short addr);
        void print_mapper(core::common::Mapper* mapper);
        void search_mapper(core::common::Mapper* mapper, pid_t &pid, int idx, u_short addr);
        void search_mapper(core::common::Mapper* mapper, std::vector<pid_t> &pids, int idx, u_short addr);
        bool delete_mapper(core::common::Mapper* mapper, int idx, int pid);
        void write_mapper(std::string filename, core::common::Mapper* mapper);
        void read_mapper(std::string filename, core::common::Mapper* mapper);
        int getTotalLine(std::string name);

        virtual bool updateStatus(bool status);
        virtual bool updateDatabase(bool status);
    };
}

