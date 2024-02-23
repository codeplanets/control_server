#pragma once

#include <regex>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>

typedef int	SOCKET;

namespace core {
    namespace system {
        class CommSock {
        private:
            int port;
            SOCKET m_sock;
            struct sockaddr_in m_addr;
            socklen_t m_len_addr;
            int m_listen_backlog;

        public:
            CommSock();
            virtual ~CommSock();

            bool create();
            bool bind(const int port);
            bool listen() const;
            bool accept(CommSock&) const;

            bool send(const char* buf, int len, int flags) const;
            int recv(char* buf, int len, int flags) const;

            void set_non_blocking(const bool);
            bool is_valid() const {return m_sock != -1; }
        };
    }
}