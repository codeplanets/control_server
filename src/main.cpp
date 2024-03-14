#include <unistd.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>

#include <cassert>

// #include <cstdio>
#include <cstdlib>      // system()
#include <csignal>     // signal(), sigaction()
#include <sys/wait.h>   // waitpid()

#include <algorithm>    // erase_if()

#include <mqueue.h>     // mq_attr, mqd_t, mq_open() , mq_close(), mq_send(), mq_receive(), mq_unlink()
#include <iostream>

#include <queue>
#include <functional>

#include "sem.h"

#include "main.h"
#include "serversocket.h"
#include "socketexception.h"

#include "connection.h"

#include "mq.h"

#include "message.h"
#include "rtu.h"
#include "cmd.h"

#include "db.h"

// #include <map>
// #include <vector>
// #include <list>

using namespace std;
using namespace core;

void sigint_handler(int signo);
void sigchld_handler(int signo);
void start_child(server::ServerSocket newSock, int pid);
void setChldSignal();
void setIntSignal();

std::vector<pid_t> connected;
std::priority_queue<u_short, vector<u_short>, less<u_short>> cmdAddrQueue;

std::string find_rtu_addr(SiteCode scode) {
    string addr = not_found;
    Database db;
    ECODE ecode = db.db_init("localhost", 3306, "rcontrol", "rcontrol2024", "RControl");
    if (ecode!= EC_SUCCESS) {
        syslog(LOG_ERR, "DB Connection Error!");
        exit(EXIT_FAILURE);
    }
    char* siteCode = scode.getSiteCode();
    string query = "select * from RSite";
    query += " where SiteCode = '";
    query += siteCode;
    query += "';";
    cout << query << endl;

    // Query Data
    MYSQL_ROW sqlrow;
    MYSQL_RES* pRes;
    ecode = db.db_query(query.c_str(), &pRes);
    if (ecode != EC_SUCCESS) {
        syslog(LOG_ERR, "DB Query Error!");
        exit(EXIT_FAILURE);
    }
    syslog(LOG_DEBUG, "+----------+----------+--------+-------+");
    syslog(LOG_DEBUG, "| SiteCode | SiteName | SiteID | Basin |");
    syslog(LOG_DEBUG, "+----------+----------+--------+-------+");
    try {
        while ((sqlrow = db.db_fetch_row(pRes)) != NULL) {
            syslog(LOG_DEBUG, "|%9s |%9s |%7s |%6s |", sqlrow[0], sqlrow[1], sqlrow[2], sqlrow[3]);
            addr = sqlrow[2];
        }
    } catch (exception& e) {
        cout << e.what() << endl;
    }
    return addr;
}

void clearMessageQueue() {
    std::system("rm -rf /dev/mqueue/rtu.*");
    std::system("rm -rf /dev/mqueue/client.*");
}

int main(int argc, char *argv[]) {
    // Log 설정
    setlogmask (LOG_UPTO (LOG_DEBUG));
    openlog("Control Server", LOG_CONS|LOG_PERROR, LOG_USER);

    syslog(LOG_INFO, "Running server!");
    syslog(LOG_DEBUG, "Setting....");

    // Zombie Process 방지 Signal 등록
    setIntSignal();
    setChldSignal();
    
    // Configuration
    int rtu_port = 5900;
    int cmd_port = 5901;

    // Set Data
    for (u_short i = 0x01; i < 0xFF; i++) {
		cmdAddrQueue.push(i);
        printf("0x%04x ", i);
	}
    printf("\n");

    // core::formatter::RSite rSite = {.status = false, .pid = 0 };
    
    if(test) {
        //////////////////////////////////////////////////////////////
        const char* site = "1000001";
        //////////////////////////////////////////////////////////////
        // message structure 변환 확인
        DATA sendbuf[MAX_RAW_BUFF] = {0x00, };

        InitReq req;
        cout << sizeof(req) << endl;
        req.fromAddr.setAddr(DEFAULT_ADDRESS);
        req.toAddr.setAddr(SERVER_ADDRESS);
        req.siteCode.setSiteCode(site);
        req.crc8.setCRC8(common::calcCRC((DATA*)&req, sizeof(req)));
        memcpy(sendbuf, (char*)&req, sizeof(req));
        common::print_hex(sendbuf, sizeof(req));

        InitRes res;
        cout << sizeof(res) << endl;
        res.fromAddr.setAddr(1, RTU_ADDRESS);
        res.toAddr.setAddr(SERVER_ADDRESS);
        res.siteCode.setSiteCode(site);
        res.rtuAddr.setAddr(1, RTU_ADDRESS);
        res.crc8.setCRC8(common::calcCRC((DATA*)&res, sizeof(res)));
        memcpy(sendbuf, (char*)&res, sizeof(res));
        common::print_hex(sendbuf, sizeof(res));

        ClientInitReq cmdreq;
        cout << sizeof(cmdreq) << endl;
        cmdreq.fromAddr.setAddr(SERVER_ADDRESS);
        cmdreq.toAddr.setAddr(DEFAULT_ADDRESS);
        cmdreq.crc8.setCRC8(common::calcCRC((DATA*)&cmdreq, sizeof(cmdreq)));
        memcpy(sendbuf, (char*)&cmdreq, sizeof(cmdreq));
        common::print_hex(sendbuf, sizeof(cmdreq));

        ClientInitRes cmdres;
        cout << sizeof(cmdres) << endl;
        cmdres.fromAddr.setAddr(SERVER_ADDRESS);
        cmdres.toAddr.setAddr(1, CLIENT_ADDRESS);
        cmdres.clientAddr.setAddr(1, CLIENT_ADDRESS);
        cmdres.crc8.setCRC8(common::calcCRC((DATA*)&cmdres, sizeof(cmdres)));
        memcpy(sendbuf, (char*)&cmdres, sizeof(cmdres));
        common::print_hex(sendbuf, sizeof(cmdres));

        //////////////////////////////////////////////////////////////
        string strAddr = find_rtu_addr(res.siteCode);
        cout << strAddr << endl;
        if (strAddr == not_found) {
            strAddr = "0";
        }
        
        //////////////////////////////////////////////////////////////
        DATA ch[2];
        // u_short num으로 형변환 
        unsigned short num = stoi(strAddr);
        // 0x0001 으로 변환됨
        printf("hex : 0x%04x, %u \n", num, num);
        // u_short 을 char[] 변환, endian 변환
        ch[0]=(char)(num >> 8) | RTU_ADDRESS; // | 0x10 주의
        ch[1]=(char)(num & 0x00ff);
        printf("0x%02x, 0x%02x \n", ch[0], ch[1]);
        res.rtuAddr.setAddr(ch, sizeof(ch));
        printf("0x%04x, %u \n", res.rtuAddr.getAddr(), res.rtuAddr.getAddr());
        res.crc8.setCRC8(common::calcCRC((DATA*)&res, sizeof(res)));
        memcpy(sendbuf, (char*)&res, sizeof(res));
        common::print_hex(sendbuf, sizeof(res));
        //////////////////////////////////////////////////////////////
        // message queue 송수신 확인
        Mq mq;
        mq.open(rtu_mq_name, getpid());

        cout << mq.send(sendbuf, sizeof(res)) << endl;

        DATA buf[MAX_RAW_BUFF] = {0x00,};
        cout << mq.recv(buf, sizeof(buf)) << endl;
        //////////////////////////////////////////////////////////////

        pause();
    }

    // Delete Data
    syslog(LOG_DEBUG, " : Complete!");

    // Multiple Running 방지
    if (common::isRunning() == true) {
        syslog(LOG_ERR, "Already running server!");
        exit(EXIT_SUCCESS);
    }

    clearMessageQueue();

    pid_t pid;
    try {
        server::ServerSocket rtu_server(rtu_port);
        server::ServerSocket cmd_server(cmd_port);
        syslog(LOG_DEBUG, "[Parent process] : listening.......");

        while(true) {
            common::sleep(100);
            server::ServerSocket new_sock;
            while(rtu_server.accept(new_sock) || cmd_server.accept(new_sock)) {
                
                pid = fork();

                if (pid == 0) {
                    syslog(LOG_DEBUG, "[Child process] %d << %d", getpid(), getppid());
                    rtu_server.close();
                    cmd_server.close();

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
                        for (size_t i = 0; i < connected.size(); i++) {
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
     * [v] Init Database
     * [ ] Getting ip, port....
     * [v] Clear memory
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
        
        // 메시지 타입 구분 (RTUs / Cmd Clients)
        if (data[0] == STX) {
            if (data[1] == INIT_REQ) {  // RTUs
                syslog(LOG_DEBUG, "Start RTU Init Request");
                InitReq msg;
                memcpy((void*)&msg, data, len);
                if (common::checkCRC((DATA*)&msg, sizeof(msg), msg.crc8.getCRC8()) == false) {
                    syslog(LOG_WARNING, "CRC Check Failed. : 0x%02X != 0x%02X", common::calcCRC((DATA*)&msg, sizeof(msg)), msg.crc8.getCRC8());
                }
                msg.print();

                RTUclient rtu(newSock);
                rtu.init(msg);
                rtu.setTimeout();
                rtu.run();

            } else if (data[1] == CLIENT_INIT_REQ) {    // Cmd Clients
                syslog(LOG_DEBUG, "Client Init Request");
                // TODO : CommandClients
                ClientInitReq msg;
                memcpy((void*)&msg, data, len);
                if (common::checkCRC((DATA*)&msg, sizeof(msg), msg.crc8.getCRC8()) == false) {
                    syslog(LOG_WARNING, "CRC Check Failed. : 0x%02X != 0x%02X", common::calcCRC((DATA*)&msg, sizeof(msg)), msg.crc8.getCRC8());
                }
                msg.print();

                CMDclient cmd(newSock);
                // Generate Client Address
                u_short cmdAddr = 1;
                if (!cmdAddrQueue.empty()) {
                    cmdAddr = cmdAddrQueue.top();
                    cmdAddrQueue.pop();
                } else {
                    syslog(LOG_ERR, "No more addresses to use.");
                }
                cmd.init(msg, cmdAddr);
                cmd.setTimeout();
                cmd.run();

            } else {
                syslog(LOG_ERR, "Unknown message type");
                common::sleep(500);

                return;
            }
        }
    }
}

// void setQuitSignal() {
//     struct sigaction act;
//     act.sa_handler = sigquit_handler;
//     sigemptyset(&act.sa_mask);
//     act.sa_flags = 0;
//     sigaction(SIGQUIT, &act, 0);
// }
// void sigquit_handler(int signo) {
//     int status;
//     pid_t spid;
//     spid = wait(&status);
//     syslog(LOG_DEBUG, "Interactive attention signal. (Quit signal)");
//     syslog(LOG_DEBUG, "PID         : %d", spid);
//     syslog(LOG_DEBUG, "Exit Value  : %d", WEXITSTATUS(status));
//     syslog(LOG_DEBUG, "Exit Stat   : %d", WIFEXITED(status));
//     syslog(LOG_DEBUG, "Exit Signal : %d", WTERMSIG(status));
//     syslog(LOG_DEBUG, "Exit Core   : %d", WCOREDUMP(status));
//     syslog(LOG_DEBUG, "Exit Status : %d", WIFSIGNALED(status));
//     common::close_sem();
//     closelog();
//     exit(EXIT_SUCCESS);
// }

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
        
        string file = "/dev/mqueue/rtu.";
        file.append(std::to_string(spid));
        unlink(file.c_str());
        
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

    // string file = "/dev/mqueue/rtu.";
    // file.append(std::to_string(getpid()));
    // unlink(file.c_str());
    
    common::close_sem();
    closelog();
    exit(EXIT_SUCCESS);
}

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
