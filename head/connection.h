#pragma once

#include "serversocket.h"

namespace core {
    namespace server {
        class Connection {
        protected:
		    ServerSocket* m_pSocket;
            pid_t m_pid;

        public:
			Connection(pid_t pid, ServerSocket* pSocket);
            virtual ~Connection();
            virtual ServerSocket* get_sock(void) { return m_pSocket; }
            virtual pid_t get_pid(void) { return m_pid; }
        };
    }
}