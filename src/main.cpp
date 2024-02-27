#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <cstdio>
#include <cstdlib>
#include <csignal>     // signal(), sigaction()
#include <sys/wait.h>   // waitpid()

#include <mqueue.h>     // mq_attr, mqd_t, mq_open() , mq_close(), mq_send(), mq_receive(), mq_unlink()

#include "sem.h"

#include "main.h"
#include "serversocket.h"
#include "socketexception.h"

#include "connection.h"

using namespace std;
using namespace core;

const int max_pool = 50;
const int listen_backlog = 5;

void sigint_handler(int signo);
void sigchld_handler(int signo);
using namespace core::server;
void start_child(server::ServerSocket newSock, int pid);
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
        server::ServerSocket server(port);
        std::cout << "[TCP server] : [Parent process] : listening......." << endl;

        while(true) {
            sleep(1);
            server::ServerSocket new_sock;
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

const char* name_posix_mq = "/mq_";

std::string get_mq_name(const char* name, int pid) {
    string mq_name;
    mq_name.append(name);
    mq_name.append(std::to_string(pid));
    cout << mq_name << endl;
    return mq_name.c_str();
}

void start_child(server::ServerSocket newSock, int pid) {
    cout << "[" << getpid() << "] : " << "Start Child Process" << endl;

    // init 정보 수신
    DATA data[MAX_RAW_BUFF];
    newSock >> data;
    cout << "[" << getpid() << "] : " << data << endl;

    // message queue 설정
    struct mq_attr mq_attrib = {.mq_maxmsg = 10, .mq_msgsize = 1024};
    cout << "[" << getpid() << "] : " << "Create MQ......" << endl;

    // message queue 이름 설정 ("mq_" + pid)
    string mqname = get_mq_name(name_posix_mq, pid);
    cout << "[" << getpid() << "] : " << mqname.c_str() << endl;

    // message queue 생성
    mqd_t mq_fd = ::mq_open(mqname.c_str(), O_CREAT|O_EXCL|O_RDWR, S_IRUSR|S_IWUSR, &mq_attrib);
    if (mq_fd > 0) {
        cout << "[" << getpid() << "] : " << "Create Message Queue[" << mqname <<"] OK!" << endl;
    } else {
        // 이미 message queue가 생성되어 있으면 Open
        if (errno == EEXIST) {
            cout << "[" << getpid() << "] : " << "Exist MQ, Open......" << endl;
            mq_fd = ::mq_open(mqname.c_str(), O_RDWR);
            if (mq_fd == (mqd_t)-1) {
                cout << "[" << getpid() << "] : " << "Failed mq_open() : " << strerror(errno) << endl;
                return;
            }
            cout << "[" << getpid() << "] : " << "Open Message Queue[" << mqname <<"] OK!" << endl;
        } else {
            cout << "[" << getpid() << "] : " << "Failed mq_open() : " << strerror(errno) << endl;
            return;
        }
    }

    // message queue 송수신 확인
    const char* msg = "abcdefg";
    size_t msg_len = sizeof(msg);

    // 송신
    cout << "[" << getpid() << "] : " << "mq_send : " << msg << endl;
    mq_send(mq_fd, msg, msg_len, 0);

    // 수신
    char buf[4096];
    mq_receive(mq_fd, buf, sizeof(buf), 0);
    cout << "[" << getpid() << "] : " << "mq_receive : " << buf << endl;

    sleep(1);

    // close message queue
    if (mq_close(mq_fd) < 0) {
        cout << "[" << getpid() << "] : " << "Failed mq_close() : " << strerror(errno) << endl;
    }

    // remove message queue
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
