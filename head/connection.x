#pragma once

#include "tcpsocket.h"
#include <iostream>

namespace core {
    namespace server {
        class Connection {

            friend class Server;

        protected:
		    TcpSocket*	m_pSocket;
		    std::string m_strClientIP;

        public:
			Connection(TcpSocket* pSocket);
            virtual ~Connection();

            virtual ECODE SetAcceptedSocket(SOCKET hNewSocket) { return m_pSocket->assign(hNewSocket); }
            virtual TcpSocket* Raw(void) {	return m_pSocket; }

            virtual void OnConnect(void) = 0;
            virtual void OnClose(void) = 0;
            virtual void OnRecv(void) = 0;
        };
    }
}