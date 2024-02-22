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

using namespace std;
using namespace core;

const int max_pool = 50;
const int listen_backlog = 5;

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

int main(int argc, char *argv[]) {

    // Multiple Running 방지
    if (common::isRunning() == true) {
        cout << "Already running server!" << endl;
        exit(EXIT_SUCCESS);
    }

    cout << "Running server!" << endl;
    // Zombie Process 방지
    setIntSignal();
    setChldSignal();

    // int clientport = 5901;
    // int listener_client;
    // socklen_t len_saddr_client;
    int rtuport = 5900;
    int listener_rtu = 0;
    socklen_t len_saddr_rtu = 0;

    listener_rtu = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (listener_rtu == -1) {
        cout << "[TCP server] : [Parent process] : Fail: rtu socket()" << endl;
        // 종료전 Multiple Running 방지 해제
        common::close_sem();
        exit(EXIT_SUCCESS);
    }

    struct sockaddr_in saddr_rtu;
    len_saddr_rtu = sizeof(saddr_rtu);
    if (rtuport == 0) {
        getsockname(listener_rtu, (struct sockaddr *)&saddr_rtu, &len_saddr_rtu);
    }

    memset(&saddr_rtu, 0, len_saddr_rtu);
    saddr_rtu.sin_family = AF_INET;
    saddr_rtu.sin_addr.s_addr = htons(INADDR_ANY);
    saddr_rtu.sin_port = htons(rtuport);

    int nRet = 0;
    // 소켓 옵션 설정
    int flag = fcntl(listener_rtu, F_GETFL, 0);
    nRet = fcntl(listener_rtu, F_SETFL, flag | O_NONBLOCK);

    int optval = 1;
    if (setsockopt(listener_rtu, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        cout << "[TCP server] : [Parent process] : Fail: setsockopt()" << endl;
        close(listener_rtu);
        // 종료전 Multiple Running 방지 해제
        common::close_sem();
        exit(EXIT_SUCCESS);
    }

    // 설정한 포트번호로 바인딩
    if (bind(listener_rtu, (struct sockaddr*)&saddr_rtu, len_saddr_rtu) == -1) {
        cout << "[TCP server] : [Parent process] : Fail: bind()" << endl;
        close(listener_rtu);
        // 종료전 Multiple Running 방지 해제
        common::close_sem();
        exit(EXIT_SUCCESS);
    }
    cout << "[TCP server] : [Parent process] : Port : #" << ntohs(saddr_rtu.sin_port) << endl;

    if (listen(listener_rtu, listen_backlog) == -1) {
        cout << "[TCP server] : [Parent process] : Fail: listen()" << endl;
        close(listener_rtu);
        // 종료전 Multiple Running 방지 해제
        common::close_sem();
        exit(EXIT_SUCCESS);
    }
    cout << "[TCP server] : [Parent process] : listening......." << endl;

    /**
     * [v] Setting named semaphore ( <== MUTEX )
     * [ ] Loading ini file
     * [ ] Init Database
     * [ ] Getting ip, port....
     * [ ] Clear memory
     * [ ] Create parents server listener socket
     * ``` 
     * listener_rtu = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
     * if (listener_rtu == -1) {
     *  logError("[TCP server] : Fail: rtu socket()");
     *  exit(EXIT_SUCCESS);
     * }
     * struct sockaddr_in saddr_rtu;
     * saddr_rtu.sin_family = AF_INET;
     * saddr_rtu.sin_addr.s_addr = htons(INADDR_ANY);
     * saddr_rtu.sin_port = htons(rtuport);
     * 
     * listener_client = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
     * if (listener_client == -1) {
     *  logError("[TCP server] : Fail: client socket()");
     *  exit(EXIT_SUCCESS);
     * }
     * struct sockaddr_in saddr_client;
     * saddr_client.sin_family = AF_INET;
     * saddr_client.sin_addr.s_addr = htons(INADDR_ANY);
     * saddr_client.sin_port = htons(clientport);
     * 
     * if (bind(listener_rtu, (struct sockaddr*)&saddr_rtu, sizeof(saddr_rtu)) == -1) {
     *  logError("[TCP server] : Fail: client bind()");
     *  exit(EXIT_SUCCESS);
     * }
     * len_saddr_rtu = sizeof(saddr_rtu);
     * ```
     * 
    **/

    struct sockaddr_in connectSocket, peerSocket;
    socklen_t connectSocketLength = sizeof(connectSocket);
    pid_t pid;
    int connection_rtu = 0;

    while(1) {
        cout << ":5900" << endl; sleep(1);
        while ((connection_rtu = accept(listener_rtu, (struct sockaddr*)&connectSocket, (socklen_t *)&connectSocketLength)) >= 0) {
            // Client 정보 수집
            getpeername(connection_rtu, (struct sockaddr*)&peerSocket, &connectSocketLength);
            char peerName[sizeof(peerSocket.sin_addr) + 1] = { 0, };
            sprintf(peerName, "%s", inet_ntoa(peerSocket.sin_addr));

            // 접속이 안되었을 때는 출력 x
            if(strcmp(peerName,"0.0.0.0") != 0) {
                cout << "[TCP server] : [Parent process] : Client : " << peerName << endl;
            }

            int optval = 1;
            if (setsockopt(connection_rtu, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) == -1) {
                cout << "[TCP server] : [Parent process] : Fail: connection setsockopt()" << endl;
                close(connection_rtu);
                break;
            }

            pid = fork();

            if (pid == 0) {
                cout << "[TCP server] : [Child process] : " << pid << endl;
                close(listener_rtu);

                // start_child(listener_rtu, i);
                sleep(10);

                cout << "[TCP server] : [Child process] : disconnect client...." << endl;
                close(connection_rtu);
                return 0;

            } else if (pid == -1) {
                close(connection_rtu);
			    continue;
                cout << "[TCP server] : Failed : fork()" << endl;
            } else {
                cout << "[TCP server] : [Parent process] : " << pid << endl;
                close(connection_rtu);
            }

        } // while(accept)

    } // while(1)
    
    close(listener_rtu);
    // 종료전 Multiple Running 방지 해제
    common::close_sem();
    return 0;
}
