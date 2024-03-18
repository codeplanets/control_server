#pragma once


// #include <cstdio>
// #include <cstdlib>
// #include <netinet/in.h>
// #include <sys/socket.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <csignal>

#include "common.h"

namespace core {
    namespace server {
        class ControlServerApplication {
        protected:
            const int MAX_POOL = 50;
            const int LISTEN_BACKLOG = 5;
            std::vector<pid_t> connected;

            void sigint_handler(int signo);
            void sigchld_handler(int signo);
            void start_child(int sfd, int idx);
            void setChldSignal();
            void setIntSignal();

        public:
            ControlServerApplication();
            virtual ~ControlServerApplication();
        };
    }
}
