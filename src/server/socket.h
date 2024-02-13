#pragma once

namespace core {
    namespace server {
        class TcpSocket {
            friend class Server;
		    friend class Connection;
        protected:
		    SOCKET m_hSocket;
        public:
            TcpSocket(void);
            virtual ~TcpSocket();

            virtual ECODE assign(SOCKET hAcceptedSocket);
            virtual ECODE connect(const char* pszIP, WORD wPort, DWORD dwTimeOut);
            virtual void close(void);

            virtual ECODE send(const void* pBuff, size_t tBufSize, DWORD dwTimeOut);
            virtual ECODE recv(void* pBuff, size_t tBufSize, DWORD dwTimeOut);

        protected:
            virtual ECODE sendWorker(SOCKET hSocket, const void* pBuff, size_t tBufSize, DWORD dwTimeOut, size_t* ptSent);
            virtual ECODE recvWorker(SOCKET hSocket, void* pBuff, size_t tBufSize, DWORD dwTimeOut, size_t* ptRead);
        };
    }
}