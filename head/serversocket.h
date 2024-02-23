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
            
            const ServerSocket& operator << (const u_char*) const;
            const ServerSocket& operator >> (u_char*) const;
        };
    }
}
