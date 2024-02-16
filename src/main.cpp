#include <iostream>
#include <unistd.h>

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <semaphore.h>
#include <fcntl.h>
#define SERVER_SEMAPHORE "control_server"

using namespace std;

sem_t* gRunning = SEM_FAILED;

namespace core {
    bool isRunning() {
		bool bRet = false;
		gRunning = ::sem_open(SERVER_SEMAPHORE, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, 1);
		if (gRunning == SEM_FAILED) {
			if (errno == EEXIST) {
				bRet = true;
                // sem_close(gRunning);
                // sem_unlink(SERVER_SEMAPHORE);
			}
		}
        return bRet;
    }

    void close_sem() {
        if (gRunning != SEM_FAILED) {
            sem_close(gRunning);
            sem_unlink(SERVER_SEMAPHORE);
        }
    }
}

using namespace core;

void sigint_handler(int signo) {
   cout << "Ctrl-C!!" << endl;
}

void start_child(int sfd, int idx) {
    cout << "Start Child Process" << endl;

    int cfd, ret_len;
    socklen_t len_saddr;
    char buf[40];
    struct sockaddr_in saddr_c;
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
    int clientport = 5901;

    int listener_rtu;
    int listener_client;

    socklen_t len_saddr_rtu;
    socklen_t len_saddr_client;

    if (isRunning() == true) {
        cout << "Already running server!" << endl;
        exit(1);
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
     *  exit(1);
     * }
     * struct sockaddr_in saddr_rtu;
     * saddr_rtu.sin_family = AF_INET;
     * saddr_rtu.sin_addr.s_addr = htons(INADDR_ANY);
     * saddr_rtu.sin_port = htons(rtuport);
     * 
     * listener_client = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
     * if (listener_client == -1) {
     *  logError("[TCP server] : Fail: client socket()");
     *  exit(1);
     * }
     * struct sockaddr_in saddr_client;
     * saddr_client.sin_family = AF_INET;
     * saddr_client.sin_addr.s_addr = htons(INADDR_ANY);
     * saddr_client.sin_port = htons(clientport);
     * 
     * if (bind(listener_rtu, (struct sockaddr*)&saddr_rtu, sizeof(saddr_rtu)) == -1) {
     *  logError("[TCP server] : Fail: client bind()");
     *  exit(0);
     * }
     * len_saddr_rtu = sizeof(saddr_rtu);
     * ```
     * 
    **/

    pid_t pid = fork();

    if (pid == 0)
        cout << "[TCP server] Output from the child process." << pid << endl;
    else
        cout << "[TCP server] Output from the parent process." << pid << endl;

    close_sem();
    return 0;
}

