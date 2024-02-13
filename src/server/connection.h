#pragma once

#include "socket.h"

namespace core {
    namespace server {
        class Connection {

            friend class Server;

        protected:
		    Socket*	m_pSocket;
		    std::string m_strClientIP;

        public:
			Connection(Socket* pSocket);
            virtual ~Connection();

            virtual ECODE SetAcceptedSocket(SOCKET hNewSocket) { return m_pSocket->Assign(hNewSocket); }
            virtual Socket* Raw(void) {	return m_pSocket; }

            virtual void OnConnect(void) = 0;
            virtual void OnClose(void) = 0;
            virtual void OnRecv(void) = 0;
        };
    }
}