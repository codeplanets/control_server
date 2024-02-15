#pragma once

#include <set>
#include <vector>
#include <iostream>
#include "connection.h"
#include "syncqueue.h"

namespace core {
    namespace server {
        enum E_DISCONNECT_TYPE {
            ////////////////////////////////
            // Disconnect types
            ////////////////////////////////
            DISCONNECT_TYPE_UNDEFINED					= 0,
        };

        struct ST_SYNCSERVER_INIT {
            E_DISCONNECT_TYPE           nDisconnectType;
		    std::vector<Connection*>    connections;

            ////////////////////////////////
            // sync server init
            ////////////////////////////////
            ST_SYNCSERVER_INIT(void)
            : nDisconnectType(DISCONNECT_TYPE_UNDEFINED)
            , connections() {}
        };

        class Server {

        protected:
            ST_SYNCSERVER_INIT m_stInit;

            TSyncQueue<Connection*> m_qReady;
            TSyncQueue<Connection*> m_qDisconnected;

            std::set<Connection*> m_setConnected;

            SOCKET m_hListenSocket;

            bool m_bContinueLoop;

            ////////////////////////////////////////////////////////////////
            // server initialization
            ////////////////////////////////////////////////////////////////
            /* TODO: 
             *      CriticalSection
             *      ThreadPool
             */

        public:
            Server();
            virtual ~Server();

            virtual ECODE Startup();
            virtual void Shutdown(void);

            size_t MaxConnectionCount(void) { return m_stInit.connections.size(); }
		    size_t ConnectionCount(void) { return m_setConnected.size(); }
        };
    }
}