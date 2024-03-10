#pragma once

#include "client.h"
#include "db.h"

namespace core {

    class RTUclient : public Client {
    private:
        Address rtuAddr;
        Address serverAddr;
        SiteCode scode;

        int m_heartbeat_limits;
        std::map<std::string, std::string> sitesMap;

    public:
        RTUclient(ServerSocket& sock, int heartbeat_limits = 5);
        ~RTUclient();

        void init(InitReq &msg);
        
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
    private:
        void print_map(std::map<std::string, std::string>& m);
        void setSiteMap(std::map<std::string, std::string> &sc_map);
    };
}

