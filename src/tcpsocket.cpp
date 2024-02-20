
#include "tcpsocket.h"
#include "socket.h"
#include "log.h"

using namespace core::system;

namespace core {
    namespace server {
        TcpSocket::TcpSocket(void) {}
        TcpSocket::~TcpSocket() {}
        ECODE TcpSocket::assign(SOCKET hAcceptedSocket) {
            return EC_SUCCESS;
        }
        ECODE TcpSocket::connect(const char* pszIP, unsigned short port, unsigned int timeOut) {
            return EC_SUCCESS;
        }
        void TcpSocket::close(void) {}

        ECODE TcpSocket::send(const void* pBuff, size_t tBufSize, unsigned int timeOut) {
            return EC_SUCCESS;
        }
        ECODE TcpSocket::recv(void* pBuff, size_t tBufSize, unsigned int timeOut) {
            return EC_SUCCESS;
        }

        ECODE TcpSocket::sendWorker(size_t hSocket, const void* pBuff, size_t tBufSize, unsigned int timeOut, size_t* ptSent) {
            return EC_SUCCESS;
        }
        ECODE TcpSocket::recvWorker(size_t hSocket, void* pBuff, size_t tBufSize, unsigned int timeOut, size_t* ptRead) {
            return EC_SUCCESS;
        }
    }
}