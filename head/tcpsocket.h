#pragma once

#include "common.h"
#include "errorcode.h"

namespace core {
    namespace server {
        class TcpSocket {
            friend class Server;
		    friend class Connection;
            
        protected:
		    SOCKET m_socket;

        public:
            TcpSocket(void);
            virtual ~TcpSocket();

            virtual ECODE assign(SOCKET hAcceptedSocket);
            virtual ECODE connect(const char* pszIP, unsigned short port, unsigned int timeOut);
            virtual void close(void);

            virtual ECODE send(const void* pBuff, size_t tBufSize, unsigned int timeOut);
            virtual ECODE recv(void* pBuff, size_t tBufSize, unsigned int timeOut);

        protected:
            virtual ECODE sendWorker(size_t hSocket, const void* pBuff, size_t tBufSize, unsigned int timeOut, size_t* ptSent);
            virtual ECODE recvWorker(size_t hSocket, void* pBuff, size_t tBufSize, unsigned int timeOut, size_t* ptRead);
        };
    }
}