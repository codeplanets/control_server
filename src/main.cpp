#include <iostream>
#include <unistd.h>

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>     // signal(), sigaction()
#include <sys/wait.h>   // waitpid()

#include "sem.h"

#include <fcntl.h>      // fcntl()

#include "main.h"
#include "serversocket.h"
#include "socketexception.h"

#include "connection.h"

using namespace std;
using namespace core;
using namespace core::server;

const int max_pool = 50;
const int listen_backlog = 5;

void sigint_handler(int signo);
void sigchld_handler(int signo);
void start_child(int sfd, int idx);
void setChldSignal();
void setIntSignal();

namespace core {
    namespace server {
        ControlServerApplication::ControlServerApplication() {}
        ControlServerApplication::~ControlServerApplication() {}
    }
}

int main(int argc, char *argv[]) {

    // Multiple Running 방지
    if (common::isRunning() == true) {
        std::cout << "Already running server!" << endl;
        exit(EXIT_SUCCESS);
    }

    std::cout << "Running server!" << endl;
    std::cout << "Setting....";

    // Zombie Process 방지 Signal 등록
    setIntSignal();
    setChldSignal();
    // Configuration
    // Connect Database
    // Query Data
    std::cout << " : Complete!" << endl;

    int port = 5900;
    pid_t pid;

    vector<Connection*> connected;

    try {
        ServerSocket server(port);
        std::cout << "[TCP server] : [Parent process] : listening......." << endl;

        while(true) {
            sleep(1);
            ServerSocket new_sock;
            while(server.accept(new_sock)) {
                
                if (connected.size() > 0) {
                    cout << "===============================" << endl;
                    cout << connected.size() << endl;
                    for (int i = 0; i < connected.size(); i++) {
                        cout << connected.at(i)->get_pid() << endl;
                    }
                    cout << "===============================" << endl;
                }

                pid = fork();

                if (pid == 0) {
                    cout << "[TCP server] : [Child process]" << getpid() << "<<" << getppid() << endl;
                    server.close();

                    // start_child(listener_rtu, i);
                    
                    sleep(10);

                    cout << "[TCP server] : [Child process] : disconnect client...." << endl;
                    
                    // Fork 되면서 child에서 erase 해도 Parent에는 erase 되지 않는다.
                    // parent child 공유되는게 필요
                    // for (auto iter = connected.begin(); iter != connected.end(); iter++) {
                    //     if ((*iter)->get_pid() == getpid()) {
                    //         (*iter)->get_sock()->close();
                    //         connected.erase(iter);
                    //     }
                    // }

                    new_sock.close();
                    return 0;

                } else if (pid == -1) {
                    cout << "[TCP server] : Failed : fork()" << endl;
                    new_sock.close();
                    sleep(1);
                    continue;
                } else {
                    cout << "[TCP server] : [Parent process] : child pid = " << pid << endl;

                    // Connection *pcon = new Connection(pid, &new_sock);
                    // connected.push_back(pcon);

                    new_sock.close();
                    sleep(1);
                }
            }
        }
    } catch (SocketException& se) {
            std::cout << "Exception was caught : [" << se.code() << "]" << se.description() << endl;

            common::close_sem();
            exit(EXIT_SUCCESS);
    }
    
    /**
     * [v] Setting named semaphore ( <== MUTEX )
     * [ ] Loading ini file
     * [ ] Init Database
     * [ ] Getting ip, port....
     * [ ] Clear memory
     * [v] Create parents server listener socket
     * ``` 
     * 
    **/

    // 종료전 Multiple Running 방지 해제
    common::close_sem();
    return 0;
}

void sigint_handler(int signo) {
    int status;
    pid_t spid;

    spid = wait(&status);
    cout << endl;
    cout << "Interactive attention signal. (Ctrl+C)" << endl;
    cout << "================================" << endl;
    cout << "PID         : " << spid << endl;
    cout << "Exit Value  : " << WEXITSTATUS(status) << endl;
    cout << "Exit Stat   : " << WIFEXITED(status) << endl;

    common::close_sem();
    exit(EXIT_SUCCESS);
}

void sigchld_handler(int signo) {
    int status;
    pid_t spid;

    while ((spid = waitpid(-1, &status, WNOHANG)) > 0) {
        cout << endl;
        cout << "자식프로세스 wait한 결과" << endl;
        cout << "================================" << endl;
        cout << "PID         : " << spid << endl;
        cout << "Exit Value  : " << WEXITSTATUS(status) << endl;
        cout << "Exit Stat   : " << WIFEXITED(status) << endl;
    }
}

void start_child(int sfd, int idx) {
    cout << "Start Child Process" << endl;

    // int cfd, ret_len;
    // socklen_t len_saddr;
    // char buf[40];
    // struct sockaddr_in saddr_c;
    /**
     * [ ] accept
     * ```
     * for(;;) {
     *  cfd = accept(sfd, (struct sockaddr *)&saddr_c, &len_saddr);
     *  if (cfd == -1) {
     *      cout << "[Child] : Fail: accept()" << endl;
     *      continue;
     *  }
     *  cout << "[Child"<< idx << "] Accept about socket " << cfd << endl;
     * }
     * ```
     * [ ] recv data
     * 
    **/
}

void setChldSignal() {
    struct sigaction act;
    act.sa_handler = sigchld_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGCHLD, &act, 0);
}

void setIntSignal() {
    struct sigaction act;
    act.sa_handler = sigint_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, 0);
}
