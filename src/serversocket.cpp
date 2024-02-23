
#include "serversocket.h"
#include "commsock.h"
#include "log.h"

using namespace core::system;

namespace core {
    namespace server {
        ServerSocket::ServerSocket(int port) {}
        ServerSocket::ServerSocket(void) {}
        ServerSocket::~ServerSocket() {}
        ECODE ServerSocket::assign(SOCKET hAcceptedSocket) {
            return EC_SUCCESS;
        }
        ECODE ServerSocket::connect(const char* pszIP, unsigned short port, unsigned int timeOut) {
            return EC_SUCCESS;
        }
        void ServerSocket::close(void) {}

        ECODE ServerSocket::send(const void* pBuff, size_t tBufSize, unsigned int timeOut) {
            return EC_SUCCESS;
        }
        ECODE ServerSocket::recv(void* pBuff, size_t tBufSize, unsigned int timeOut) {
            return EC_SUCCESS;
        }

        ECODE ServerSocket::sendWorker(size_t hSocket, const void* pBuff, size_t tBufSize, unsigned int timeOut, size_t* ptSent) {
            return EC_SUCCESS;
        }
        ECODE ServerSocket::recvWorker(size_t hSocket, void* pBuff, size_t tBufSize, unsigned int timeOut, size_t* ptRead) {
            return EC_SUCCESS;
        }
    }
}