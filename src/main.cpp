#include <iostream>
#include <unistd.h>

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define MAX_POOL 3

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#define LISTEN_BACKLOG 15

#include "sem.h"

using namespace std;
using namespace core;

void sigint_handler(int signo) {
   cout << "Ctrl-C!!" << endl;
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

int main(int argc, char *argv[]) {

    int rtuport = 5900;
    // int clientport = 5901;

    int listener_rtu;
    // int listener_client;

    socklen_t len_saddr_rtu;
    // socklen_t len_saddr_client;

    if (common::isRunning() == true) {
        cout << "Already running server!" << endl;
        exit(EXIT_SUCCESS);
    }

    cout << "Running server!" << endl;
    signal(SIGINT, sigint_handler);
    
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

    listener_rtu = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (listener_rtu == -1) {
        cout << "[TCP server] : Fail: rtu socket()" << endl;
        common::close_sem();
        exit(EXIT_SUCCESS);
    }

    struct sockaddr_in saddr_rtu;
    saddr_rtu.sin_family = AF_INET;
    saddr_rtu.sin_addr.s_addr = htons(INADDR_ANY);
    saddr_rtu.sin_port = htons(rtuport);

    int optval = 1;
    setsockopt(listener_rtu, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if (bind(listener_rtu, (struct sockaddr*)&saddr_rtu, sizeof(saddr_rtu)) == -1) {
        cout << "[TCP server] : Fail: client bind()" << endl;
        common::close_sem();
        exit(EXIT_SUCCESS);
    }

    len_saddr_rtu = sizeof(saddr_rtu);
    if (rtuport == 0) {
        getsockname(listener_rtu, (struct sockaddr *)&saddr_rtu, &len_saddr_rtu);
    }

    cout << "[TCP server] : Port : #" << ntohs(saddr_rtu.sin_port) << endl;
    listen(listener_rtu, LISTEN_BACKLOG);
    cout << "[TCP server] : listen" << endl;

    for (int i = 0; i < MAX_POOL; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            cout << "[TCP server] Output from the child process." << pid << endl;
            start_child(listener_rtu, i);
        } else if (pid == -1) {
            cout << "[TCP server] : Fail : fork()" << endl;
        } else {
            cout << "[TCP server] Output from the parent process." << pid << endl;
        }
    }
    
    common::close_sem();
    return 0;
}

