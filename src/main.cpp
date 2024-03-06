#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <cassert>

#include <cstdio>
#include <cstdlib>
#include <csignal>     // signal(), sigaction()
#include <sys/wait.h>   // waitpid()

#include <algorithm>    // erase_if()

#include <mqueue.h>     // mq_attr, mqd_t, mq_open() , mq_close(), mq_send(), mq_receive(), mq_unlink()

#include "sem.h"

#include "main.h"
#include "serversocket.h"
#include "socketexception.h"

#include "connection.h"

#include "mq.h"

#include "message.h"
#include "rtu.h"

using namespace std;
using namespace core;

const int max_pool = 50;
const int listen_backlog = 5;

void sigint_handler(int signo);
void sigchld_handler(int signo);
void start_child(server::ServerSocket newSock, int pid);
void setChldSignal();
void setIntSignal();

namespace core {
    namespace server {
        ControlServerApplication::ControlServerApplication() {}
        ControlServerApplication::~ControlServerApplication() {}
        
        void ControlServerApplication::sigint_handler(int signo) {}
        void ControlServerApplication::sigchld_handler(int signo) {}
        void ControlServerApplication::start_child(int sfd, int idx) {}
        void ControlServerApplication::setChldSignal() {}
        void ControlServerApplication::setIntSignal() {}
    }
}


                                                                                                                                                                                                                                                        



vector<pid_t> connected;

int main(int argc, char *argv[]) {
    setlogmask (LOG_UPTO (LOG_DEBUG));
    openlog("Control Server", LOG_CONS|LOG_NDELAY|LOG_PERROR, LOG_USER);

    // Multiple Running 방지
    if (common::isRunning() == true) {
        syslog(LOG_ERR, "Already running server!");
        exit(EXIT_SUCCESS);
    }

    syslog(LOG_INFO, "Running server!");
    syslog(LOG_DEBUG, "Setting....");

    // Zombie Process 방지 Signal 등록
    setIntSignal();
    setChldSignal();
    // Configuration
    int port = 5900;

    // Connect Database
    // Query Data
    // Set Data
    core::formatter::RSite rSite = {.status = false, .pid = 0 };
    if(0) {
        //////////////////////////////////////////////////////////////
        DATA addr[2] = {0x00,};
        addr[0] = 0x1F;
        addr[1] = 0xFF;
        const char* site = "1234567";
        //////////////////////////////////////////////////////////////
        // message structure 변환 확인
        InitRes res;
        cout << sizeof(res) << endl;
        res.fromAddr.setAddr(addr, sizeof(addr));
        res.toAddr.setAddr(addr, sizeof(addr));
        res.siteCode.setSiteCode(site);
        res.rtuAddr.setAddr(addr, sizeof(addr));
        DATA sendbuf[sizeof(res)] = {0x00, };
        memcpy(sendbuf, (char*)&res, sizeof(res));
        common::print_hex(sendbuf, sizeof(sendbuf));

        InitRes msg;
        memcpy(&msg, sendbuf, sizeof(msg));
        msg.print();
        //////////////////////////////////////////////////////////////
        // message queue 송수신 확인
        Mq mq;
        bool isCreated = mq.open(getpid());
        cout << mq.send(sendbuf, sizeof(sendbuf)) << endl;
        DATA buf[MAX_RAW_BUFF] = {0x00,};
        cout << mq.recv(buf, sizeof(buf)) << endl;
        //////////////////////////////////////////////////////////////

        pause();
    }

    // Delete Data
    syslog(LOG_DEBUG, " : Complete!");

    pid_t pid;

    try {
        server::ServerSocket server(port);
        syslog(LOG_DEBUG, "[Parent process] : listening.......");

        while(true) {
            common::sleep(1000);
            server::ServerSocket new_sock;
            while(server.accept(new_sock)) {
                
                pid = fork();

                if (pid == 0) {
                    syslog(LOG_DEBUG, "[Child process] %d << %d", getpid(), getppid());
                    server.close();

                    start_child(new_sock, getpid());
                    
                    common::sleep(500);

                    syslog(LOG_DEBUG, "[%d] : disconnect client....", getpid());
                    
                    new_sock.close();
                    return 0;

                } else if (pid == -1) {
                    syslog(LOG_ERR, "[Error : %s:%d] Failed : fork() : %s",__FILE__, __LINE__, strerror(errno));
                    new_sock.close();
                    common::sleep(500);
                    continue;
                } else {
                    syslog(LOG_DEBUG, "[Parent process] : child pid = %d", pid);

                    connected.push_back(pid);

                    if (connected.size() > 0) {
                        syslog(LOG_DEBUG, "Child Count : %ld", connected.size());
                        for (int i = 0; i < connected.size(); i++) {
                            syslog(LOG_DEBUG, " -pid : %d", connected.at(i));
                        }
                    }
                    new_sock.close();
                    common::sleep(500);
                }
            }
        }
    } catch (SocketException& se) {
            // std::cout << "Exception was caught : [" << se.code() << "]" << se.description() << endl;
            syslog(LOG_CRIT, "[Error : %s:%d] Exception was caught : [%d] %s",__FILE__, __LINE__, se.code(), se.description().c_str());
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

void start_child(server::ServerSocket newSock, int pid) {
    syslog(LOG_DEBUG, "Start Child Process");

    // init 정보 수신
    DATA data[MAX_RAW_BUFF] = { 0 };

    int len = newSock.recv(data, MAX_RAW_BUFF);

    if (len > 0) {
        cout << "[" << getpid() << "] : " << endl;
        core::common::print_hex(data, len);
        
        // 메시지 타입 구분
        if (data[0] == STX) {
            if (data[1] == INIT_REQ) {  // RTU
                syslog(LOG_DEBUG, "Start RTU Init Request");
                InitReq msg;
                memcpy(&msg, data, len);
                msg.print();

                RTUclient rtu;
                rtu.init(newSock);
                rtu.run();

            } else if (data[1] == CLIENT_INIT_REQ) {    // CmdClients
                syslog(LOG_DEBUG, "Client Init Request");
                // TODO : CommandClients
            } else {
                syslog(LOG_ERR, "Unknown message type");
                common::sleep(500);

                return;
            }
        }
    }
    
}

void setChldSignal() {
    struct sigaction act;
    act.sa_handler = sigchld_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGCHLD, &act, 0);
}

void sigchld_handler(int signo) {
    int status;
    pid_t spid;

    while ((spid = waitpid(-1, &status, WNOHANG)) > 0) {
        syslog(LOG_DEBUG, "자식프로세스 wait한 결과");
        syslog(LOG_DEBUG, "PID         : %d", spid);
        syslog(LOG_DEBUG, "Exit Value  : %d", WEXITSTATUS(status));
        syslog(LOG_DEBUG, "Exit Stat   : %d", WIFEXITED(status));
        syslog(LOG_DEBUG, "Child Count : %ld", connected.size());
        std::erase_if(connected, [&spid](const auto& ele) {
            return ele == spid;
            }
        );
        syslog(LOG_DEBUG, "Child Count : %ld", connected.size());
    }
}

void setIntSignal() {
    struct sigaction act;
    act.sa_handler = sigint_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, 0);
}

void sigint_handler(int signo) {
    int status;
    pid_t spid;

    spid = wait(&status);
    syslog(LOG_DEBUG, "Interactive attention signal. (Ctrl+C)");
    syslog(LOG_DEBUG, "PID         : %d", spid);
    syslog(LOG_DEBUG, "Exit Value  : %d", WEXITSTATUS(status));
    syslog(LOG_DEBUG, "Exit Stat   : %d", WIFEXITED(status));
    common::close_sem();
    closelog();
    exit(EXIT_SUCCESS);
}
