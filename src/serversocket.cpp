
#include "serversocket.h"
#include "commsock.h"
// #include "log.h"
#include "errorcode.h"
#include "socketexception.h"

using namespace std;
using namespace core::system;

namespace core {
    namespace server {
        ServerSocket::ServerSocket(int port) {
            if (!CommSock::create()) {
                throw SocketException(EC_NOT_CREATED, "Could not create server socket.");
            }
            syslog(LOG_DEBUG, "create socket()");

            CommSock::set_non_blocking(true);

            if (!CommSock::bind(port)) {
                throw SocketException(EC_BIND_FAILURE, "Could not bind to port.");
            }
            syslog(LOG_DEBUG, "bind()");

            if (!CommSock::listen()) {
                throw SocketException(EC_LISTEN_FAILURE, "Could not listen to socket.");
            }
            syslog(LOG_DEBUG, "listen()");
        }
        
        ServerSocket::~ServerSocket() { }

        bool ServerSocket::accept(ServerSocket& sock) {
            return CommSock::accept(sock);
        }

        void ServerSocket::close(void) {
            CommSock::close();
        }

        int ServerSocket::send(const DATA* s, size_t len) {
            return CommSock::send(s, len);
        }

        int ServerSocket::recv(DATA* r, size_t len) {
            return CommSock::recv(r, len);
        }

        int ServerSocket::peek(DATA* r, size_t len) {
            return CommSock::recv(r, len, MSG_PEEK|MSG_DONTWAIT);
        }
    }
}
