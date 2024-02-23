
#include "serversocket.h"
#include "commsock.h"
#include "log.h"
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
            cout << "[ServerSocket] : create socket()" << endl;

            CommSock::set_non_blocking(true);

            if (!CommSock::bind(port)) {
                throw SocketException(EC_BIND_FAILURE, "Could not bind to port.");
            }
            cout << "[ServerSocket] : bind()" << endl;

            if (!CommSock::listen()) {
                throw SocketException(EC_LISTEN_FAILURE, "Could not listen to socket.");
            }
            cout << "[ServerSocket] : listen()" << endl;
        }
        
        ServerSocket::~ServerSocket() { }

        bool ServerSocket::accept(ServerSocket& sock) {
            return CommSock::accept(sock);
        }

        void ServerSocket::close(void) {
            CommSock::close();
        }

        const ServerSocket& ServerSocket::operator << (const u_char* s) const {
            if (!CommSock::send(s, sizeof(s))) {
                throw SocketException(EC_WRITE_FAILURE, "Could not write to socket.");
            }
            return *this;
        }

        const ServerSocket& ServerSocket::operator >> (u_char* r) const {
            if (!CommSock::recv(r, sizeof(r))) {
                throw SocketException(EC_READ_FAILURE, "Could not read from socket.");
            }
            return *this;
        }
    }
}
