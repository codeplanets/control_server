
#include <iostream>
#include <string.h>
#include <errno.h>
#include <fcntl.h>  // fcntl()

#include "commsock.h"

using namespace std;

namespace core {
    namespace system {
        CommSock::CommSock(void)
         : m_sock(-1)
         , m_len_addr(0)
         , m_listen_backlog(5) {
            memset(&m_addr, 0, sizeof(m_addr));
        }

        CommSock::~CommSock() {
            if(is_valid()) {
                ::close(m_sock);
            }
        }

        bool CommSock::create(void) {
            m_sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
            if(!is_valid()) {
                syslog(LOG_ERR, "[%s:%d]  : Failed create socket()", __FILE__, __LINE__);
                return false;
            }
            // TIME_WAIT - argh
            int reuse_on = 1;
            if(setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse_on, sizeof(reuse_on)) == -1) {
                syslog(LOG_ERR, "[%s:%d]  : Failed setsockopt(REUSEADDR)", __FILE__, __LINE__);
                return false;
            }
            // Keep alive
            int keepalive_on = 1;
            if(setsockopt(m_sock, SOL_SOCKET, SO_KEEPALIVE, (const char*)&keepalive_on, sizeof(keepalive_on)) == -1) {
                syslog(LOG_ERR, "[%s:%d]  : Failed setsockopt(KEEPALIVE)", __FILE__, __LINE__);
                return false;
            }

            return true;
        }

        bool CommSock::bind(const int port) {
            if(!is_valid()) {
                return false;
            }

            m_addr.sin_family = AF_INET;
            m_addr.sin_addr.s_addr = INADDR_ANY;
            m_addr.sin_port = htons(port);
            socklen_t len_addr = sizeof(m_addr);

            int bind_return = ::bind(m_sock, (struct sockaddr *)&m_addr, len_addr);
            if(bind_return == -1) {
                syslog(LOG_ERR, "[%s:%d]  : Failed bind()", __FILE__, __LINE__);
                return false;
            }

            return true;
        }

        bool CommSock::listen(void) const {
            if(!is_valid()) {
                return false;
            }

            int listen_return = ::listen(m_sock, m_listen_backlog);
            if(listen_return == -1) {
                syslog(LOG_ERR, "[%s:%d]  : Failed listen()", __FILE__, __LINE__);
                return false;
            }

            return true;
        }

        bool CommSock::accept(CommSock& new_socket) const {
            socklen_t len_addr = sizeof(m_addr);
            new_socket.m_sock = ::accept(m_sock, (sockaddr *)&m_addr, &len_addr);

            if(new_socket.m_sock <= 0) {
                return false;
            } else {
                return true;
            }
        }

        void CommSock::close(void) {
            if(is_valid()) {
                ::close(m_sock);
            }
        }

        bool CommSock::send(const DATA* buf, size_t len, int flags) const {
            ssize_t status = ::send(m_sock, buf, len, flags);
            if(status == -1) {
                syslog(LOG_ERR, "[%s:%d-%s] : status == -1 errno == %d", __FILE__, __LINE__, __FUNCTION__, errno);
            }
            return status;
        }

        int CommSock::recv(DATA* buf, size_t len, int flags) const {
            memset(buf, 0, len + 1);
            ssize_t status = ::recv(m_sock, buf, len, flags);
            if(status == -1) {
                syslog(LOG_ERR, "[%s:%d-%s] : status == -1 errno == %d", __FILE__, __LINE__, __FUNCTION__, errno);
            }
            return status;
        }

        void CommSock::set_non_blocking (const bool b) {
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
