#pragma once

#include <set>
#include <vector>
#include <iostream>
#include "connection.h"
#include "syncqueue.h"
#include "threadpool.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>

using namespace core::common;

namespace core {
    namespace server {
        enum E_DISCONNECT_TYPE {
            ////////////////////////////////
            // Disconnect types
            ////////////////////////////////
            DISCONNECT_TYPE_UNDEFINED					= 0,
            DISCONNECT_TYPE_NO_RESPONSE_ON_CONNECT		,	// pending count=0,
            DISCONNECT_TYPE_CONNECTED_AND_NO_RESPONSE	,	// pending count=100, no accept
            DISCONNECT_TYPE_CONNECTED_AND_CLOSE			,	// pending count=100, accept and close
        };

        struct ST_SYNCSERVER_INIT {
            int port;
            int reserved;
            int recvTimeout;
            E_DISCONNECT_TYPE           nDisconnectType;
		    std::vector<Connection*>    connections;

            ////////////////////////////////
            // sync server init
            ////////////////////////////////
            ST_SYNCSERVER_INIT(void)
            : port(0), reserved(0), recvTimeout(10000)
            , nDisconnectType(DISCONNECT_TYPE_NO_RESPONSE_ON_CONNECT)
            , connections() {}
        };

        class Server {

        protected:
            ST_SYNCSERVER_INIT m_stInit;

            TSyncQueue<Connection*> m_qReady;
            TSyncQueue<Connection*> m_qDisconnected;

            std::set<Connection*> m_setConnected;
            ThreadPool m_ThreadPool;

            SOCKET m_listenSocket;

            void* m_listenThread;
            void* m_disconnectThread;

            bool m_bContinueLoop;

            ////////////////////////////////////////////////////////////////
            // server initialization
            ////////////////////////////////////////////////////////////////
            /* TODO: 
             *      CriticalSection
             */

        public:
            Server();
            virtual ~Server();

            virtual ECODE Startup(const ST_SYNCSERVER_INIT& stInit);
            virtual void Shutdown(void);

            size_t MaxConnectionCount(void) { return m_stInit.connections.size(); }
		    size_t ConnectionCount(void) { return m_setConnected.size(); }

        private:
            int			ListenThread(void * pContext);
            int			DisconnectThread(void * pContext);

            friend int	ConnectionThreadCaller(void* pContext);
            void		ConnectionThread(Connection* pConnection);

        };
    }
}