#pragma once

#include "common.h"
#include "errorcode.h"
#include "commsock.h"

namespace core {
    namespace server {
        class ServerSocket : private core::system::CommSock {
        public:
            ServerSocket(void) {}
            ServerSocket(int port);
            virtual ~ServerSocket();

            bool accept(ServerSocket&);
            void close(void);
            
            int send(const DATA* r, size_t len);
            int recv(DATA* r, size_t len);
        };
    }
}
