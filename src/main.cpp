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
#include <mqueue.h>     // mq_open()

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
void start_child(ServerSocket newSock, int pid);
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
                    cout << "[TCP server] : [Child process]" << getpid() << " << " << getppid() << endl;
                    server.close();

                    start_child(new_sock, getpid());
                    
                    sleep(10);

                    cout << "[" << getpid() << "] : " << "disconnect client...." << endl;
                    
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
                    cout << "[TCP server] : Failed : fork() : " << strerror(errno) << endl;
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

const char* name_posix_mq = "/my_mq_";

std::string get_mq_name(const char* name, int pid) {
    string mq_name;
    mq_name.append(name);
    mq_name.append(std::to_string(pid));
    cout << mq_name << endl;
    return mq_name.c_str();
}

void start_child(ServerSocket newSock, int pid) {
    cout << "[" << getpid() << "] : " << "Start Child Process" << endl;

    // init 정보 수신
    u_char data[MAX_RAW_BUFF];
    newSock >> data;
    cout << "[" << getpid() << "] : " << data << endl;

    struct mq_attr mq_attrib = {.mq_maxmsg = 10, .mq_msgsize = 1024};
    cout << "[" << getpid() << "] : " << "Create MQ......" << endl;

    string mqname = get_mq_name(name_posix_mq, pid);
    cout << "[" << getpid() << "] : " << mqname.c_str() << endl;

    mqd_t mq_fd = ::mq_open(mqname.c_str(), O_CREAT|O_EXCL|O_RDWR, S_IRUSR|S_IWUSR, &mq_attrib);
    if (mq_fd > 0) {
        cout << "[" << getpid() << "] : " << "OK!" << endl;
    } else {
        if (errno == EEXIST) {
            cout << "[" << getpid() << "] : " << "Open MQ......" << endl;
            mq_fd = ::mq_open(mqname.c_str(), O_RDWR);
            if (mq_fd == (mqd_t)-1) {
                cout << "[" << getpid() << "] : " << "Failed mq_open() : " << strerror(errno) << endl;
                return;
            }
            cout << "[" << getpid() << "] : " << "Open MQ" << endl;
        } else {
            cout << "[" << getpid() << "] : " << "Failed mq_open() : " << strerror(errno) << endl;
            return;
        }
    }

    const char* msg = "abcdefg";
    size_t msg_len = sizeof(msg);

    cout << "[" << getpid() << "] : " << "mq_send : " << msg << endl;
    mq_send(mq_fd, msg, msg_len, 0);

    char buf[4096];
    mq_receive(mq_fd, buf, sizeof(buf), 0);
    cout << "[" << getpid() << "] : " << "mq_receive : " << buf << endl;

    sleep(1);

    if (mq_close(mq_fd) < 0) {
        cout << "[" << getpid() << "] : " << "Failed mq_close() : " << strerror(errno) << endl;
    }

    if (mq_unlink(mqname.c_str()) < 0) {
        cout << "[" << getpid() << "] : " << "Failed mq_unlink() : " << strerror(errno) << endl;
    }
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
