
#include <iostream>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "sock.h"

using namespace std;

namespace core {
    namespace system {
        socket::socket()
         : m_sock(-1) {
            memset(&m_addr, 0, sizeof(m_addr));
        }

        socket::~socket() {
            if(is_valid()) {
                ::close(m_sock);
            }
        }

        bool socket::create() {
            m_sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
            if(!is_valid()) {
                return false;
            }
            // TIME_WAIT - argh
            int reuse_on = 1;
            if(setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse_on, sizeof(reuse_on)) == -1) {
                return false;
            }
            // Keep alive
            int keepalive_on = 1;
            if(setsockopt(m_sock, SOL_SOCKET, SO_KEEPALIVE, (const char*)&keepalive_on, sizeof(keepalive_on)) == -1) {
                return false;
            }

            return true;
        }

        bool socket::bind(const int port) {
            if(!is_valid()) {
                return false;
            }
            m_addr.sin_family = AF_INET;
            m_addr.sin_addr.s_addr = INADDR_ANY;
            m_addr.sin_port = htons(port);

            int bind_return = ::bind(m_sock, (struct sockaddr *)&m_addr, sizeof(m_addr));
            if(bind_return == -1) {
                return false;
            }

            return true;
        }

        bool socket::listen() const {
            if(!is_valid()) {
                return false;
            }

            int listen_return = ::listen(m_sock, 100);
            if(listen_return == -1) {
                return false;
            }

            return true;
        }

        bool socket::accept(socket& new_socket) const {
            int addr_length = sizeof(m_addr);
            new_socket.m_sock = ::accept(m_sock, (sockaddr *)&m_addr, (socklen_t *)&addr_length);

            if(new_socket.m_sock <= 0) {
                return false;
            } else {
                return true;
            }
        }

        bool socket::send(const char* buf, int len, int flags) const {
            int status = ::send(m_sock, buf, len, flags);
            if(status == -1) {
                return false;
            } else {
                return true;
            }
        }

        int socket::recv(char* buf, int len, int flags) const {
            memset(buf, 0, len + 1);
            int status = ::recv(m_sock, buf, len, flags);
            if(status == -1) {
                cout << "status == -1   errno == " << errno << "  in core::system::socket::recv" << endl;
                return 0;
            } else if(status == 0) {
                return 0;
            } else {
                return status;
            }
        }

        void socket::set_non_blocking (const bool b) {
            int opts = fcntl(m_sock, F_GETFL);
            if(opts < 0) {
                return;
            }

            if(b) {
                opts = (opts | O_NONBLOCK);
            } else {
                opts = (opts & ~O_NONBLOCK);
            }

            fcntl(m_sock, F_SETFL, opts);
        }
    }
}
