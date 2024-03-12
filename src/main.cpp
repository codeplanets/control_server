#include <unistd.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>

#include <cassert>

// #include <cstdio>
// #include <cstdlib>
#include <csignal>     // signal(), sigaction()
#include <sys/wait.h>   // waitpid()

#include <algorithm>    // erase_if()

#include <mqueue.h>     // mq_attr, mqd_t, mq_open() , mq_close(), mq_send(), mq_receive(), mq_unlink()
#include <iostream>

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

#include <map>
// #include <vector>
// #include <list>

using namespace std;
using namespace core;

std::map<string, string> sitesMap;

void sigint_handler(int signo);
void sigchld_handler(int signo);
void start_child(server::ServerSocket newSock, int pid);
void setChldSignal();
void setIntSignal();

std::vector<pid_t> connected;
void print_map(std::map<std::string, std::string> &m) {
    for (std::map<std::string, std::string>::iterator itr = m.begin(); itr != m.end(); ++itr) {
        std::cout << itr->first << " " << itr->second << std::endl;
    }
}

void setSiteMap(map<std::string, std::string> &sc_map) {
    sc_map.clear();

    Database db;
    ECODE ecode = db.db_init("localhost", 3306, "rcontrol", "rcontrol2024", "RControl");
    if (ecode!= EC_SUCCESS) {
        syslog(LOG_ERR, "DB Connection Error!");
        exit(EXIT_FAILURE);
    }

    // Query Data
    MYSQL_ROW sqlrow;
    MYSQL_RES* pRes;
    ecode = db.db_query("select * from RSite", &pRes);
    if (ecode!= EC_SUCCESS) {
        syslog(LOG_ERR, "DB Query Error!");
        exit(EXIT_FAILURE);
    }

    syslog(LOG_DEBUG, "+----------+----------+--------+-------+");
    syslog(LOG_DEBUG, "| SiteCode | SiteName | SiteID | Basin |");
    syslog(LOG_DEBUG, "+----------+----------+--------+-------+");
    try {
        while ((sqlrow = db.db_fetch_row(pRes)) != NULL) {
            syslog(LOG_DEBUG, "|%9s |%9s |%7s |%6s |", sqlrow[0], sqlrow[1], sqlrow[2], sqlrow[3]);
            sc_map[sqlrow[0]] = sqlrow[2];
            
            if (sc_map.empty()) { cout << "map is empty." << endl; }
            else cout << "map is not empty : " << sc_map.size() << endl;

            common::sleep(1000);
        }
    } catch (exception& e) {
        cout << e.what() << endl;
    }
    
    syslog(LOG_DEBUG, "+----------+----------+--------+-------+");
    // print_map(sc_map);
    // cout << sc_map.find("2000004")->second << endl;   // 14
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
    int port = 5900;

    // Connect Database
    setSiteMap(sitesMap);
    print_map(sitesMap);
    
    // Set Data
    
    // core::formatter::RSite rSite = {.status = false, .pid = 0 };
    
    if(test) {
        //////////////////////////////////////////////////////////////
        DATA addr[2] = {0x00,};
        addr[0] = 0x1F;
        addr[1] = 0xFF;
        const char* site = "1000007";
        //////////////////////////////////////////////////////////////
        // message structure 변환 확인
        InitReq req;
        InitRes res;
        cout << sizeof(res) << endl;
        res.fromAddr.setAddr(addr, sizeof(addr));
        res.toAddr.setAddr(addr, sizeof(addr));
        res.siteCode.setSiteCode(site);
        res.rtuAddr.setAddr(addr, sizeof(addr));
        res.crc8.setCRC8(0x00);
        DATA sendbuf[sizeof(res)] = {0x00, };
        memcpy(sendbuf, (char*)&res, sizeof(res));
        res.crc8.setCRC8(common::calcCRC(sendbuf, sizeof(sendbuf) - 2));
        memcpy(sendbuf, (char*)&res, sizeof(res));
        common::print_hex(sendbuf, sizeof(sendbuf));

        char* siteCode = res.siteCode.getSiteCode();
        cout << siteCode << endl;
        string strAddr = sitesMap.find(siteCode)->second;// 7
        delete siteCode;

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
        memcpy(sendbuf, (char*)&res, sizeof(res));

        res.crc8.setCRC8(common::calcCRC(sendbuf, sizeof(sendbuf) - 2));
        memcpy(sendbuf, (char*)&res, sizeof(res));
        common::print_hex(sendbuf, sizeof(sendbuf));

        //////////////////////////////////////////////////////////////
        InitRes msg;
        memcpy((void*)&msg, sendbuf, sizeof(msg));
        msg.crc8.setCRC8(common::calcCRC(sendbuf, sizeof(sendbuf) - 2));
        msg.print();
        //////////////////////////////////////////////////////////////
        // message queue 송수신 확인
        Mq mq;
        mq.open(rtu_mq_name, getpid());
        cout << mq.send(sendbuf, sizeof(sendbuf)) << endl;
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
        
        // 메시지 타입 구분 (RTUs / Cmd Clients)
        if (data[0] == STX) {
            if (data[1] == INIT_REQ) {  // RTUs
                syslog(LOG_DEBUG, "Start RTU Init Request");
                InitReq msg;
                memcpy((void*)&msg, data, len);
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
                msg.print();

                CMDclient cmd(newSock);
                // Generate Client Address
                string cmdAddr = "1";
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

    string file = "/dev/mqueue/rtu.";
    file.append(std::to_string(getpid()));
    unlink(file.c_str());
    
    common::close_sem();
    cout << "0" << endl;
    closelog();
    cout << "1" << endl;
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
