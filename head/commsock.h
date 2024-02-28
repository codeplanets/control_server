#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "common.h"

namespace core {
    namespace system {
        class CommSock {
        private:
            SOCKET m_sock;
            struct sockaddr_in m_addr;
            socklen_t m_len_addr;
            int m_listen_backlog;

        public:
            CommSock(void);
            virtual ~CommSock();

            bool create(void);
            bool bind(const int port);
            bool listen(void) const;
            bool accept(CommSock&) const;
            void close(void);

            bool send(const DATA* buf, size_t len, int flags=0) const;
            int recv(DATA* buf, size_t len, int flags=0) const;

            void set_non_blocking(const bool);
            bool is_valid(void) const {return m_sock != -1; }
        };
    }
}