#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
 
namespace core {
    namespace server {
        class ControlServerApplication {
        protected:
            const int max_pool = 50;
            const int listen_backlog = 5;

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
