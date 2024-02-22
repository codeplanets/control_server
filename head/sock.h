#pragma once

#include <regex>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>

typedef size_t				SOCKET;

namespace core {
    namespace system {
        class socket {
        private:
            SOCKET m_sock;
            sockaddr_in m_addr;

        public:
            socket();
            virtual ~socket();

            bool create();
            bool bind(const int port);
            bool listen() const;
            bool accept(socket&) const;

            bool send(const char* buf, int len, int flags) const;
            int recv(char* buf, int len, int flags) const;

            void set_non_blocking(const bool);
            bool is_valid() const {return m_sock != -1; }
        };
    }
}