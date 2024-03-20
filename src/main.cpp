#include <unistd.h>
#include <cassert>
#include <cstdlib>      // system()
#include <csignal>     // signal(), sigaction()
#include <sys/wait.h>   // waitpid()
#include <algorithm>    // erase_if()
#include <mqueue.h>     // mq_attr, mqd_t, mq_open() , mq_close(), mq_send(), mq_receive(), mq_unlink()
#include <queue>
#include <functional>
#include <vector>
#include <iostream>
#include <filesystem>
#include <set>

#include "main.h"
#include "serversocket.h"
#include "socketexception.h"
#include "connection.h"
#include "sem.h"
#include "mq.h"
#include "message.h"
#include "rtu.h"
#include "cmd.h"
#include "db.h"

using namespace std;
using namespace core;

void sigint_handler(int signo);
void sigchld_handler(int signo);
void start_child(server::ServerSocket newSock, int pid, u_short cmdAddr = 1);
void setChldSignal();
void setIntSignal();
///////////////////////////////////////////////////////////////////////////////////
std::vector<pid_t> connected;
std::priority_queue<u_short, vector<u_short>, greater<u_short>> cmdAddrQueue;

std::string find_rtu_addr(SiteCode scode) {
    string addr = NOT_FOUND;
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
    syslog(LOG_DEBUG, "Query : %s", query.c_str());
    delete[] siteCode;

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
        syslog(LOG_ERR, "DB Fetch Error!");
        cout << e.what() << endl;
    }
    syslog(LOG_DEBUG, "+----------+----------+--------+-------+");
    return addr;
}

void clearMessageQueue() {
    std::system("rm -rf /dev/mqueue/rtu.*");
    std::system("rm -rf /dev/mqueue/client.*");
}

pid_t find_rtu_pid(core::common::MAPPER* list, int idx, u_short addr) {
    core::common::MAPPER* map;
    for (map = list; map < list + idx; map++) {
        if (map->addr == addr) {
            return map->pid;
        }
    }
    return -1;
}

core::common::MAPPER rtu_mapper_list[MAX_POOL] = {0, };

void createDataFile(std::string filename) {
    FILE * f = fopen(filename.c_str(), "w");
    fclose(f);
}

core::common::MAPPER add_mapper(int pid, u_short addr) {
    core::common::MAPPER mapper;
    mapper.pid = pid;
    mapper.addr = addr;
    return mapper;
}

void print_mapper(core::common::MAPPER* mapper) {
    for (int i = 0; i < MAX_POOL; i++) {
        if (mapper[i].pid != 0) {
            syslog(LOG_DEBUG, "Mapper : %d 0x%02X", mapper[i].pid, mapper[i].addr);
        }
    }
}

void search_mapper(core::common::MAPPER* mapper, pid_t &pid, int idx, u_short addr) {
    core::common::MAPPER* map;
    for (map = mapper; map < mapper + idx; map++) {
        if (map->addr == addr) {
            syslog(LOG_DEBUG, "Found Mapper : %d 0x%02X", map->pid, map->addr);

            pid = map->pid;
        }
    }
}

void search_mapper(core::common::MAPPER* mapper, std::vector<pid_t> &pids, int idx, u_short addr) {
    core::common::MAPPER* map;
    for (map = mapper; map < mapper + idx; map++) {
        if (map->addr == addr) {
            syslog(LOG_DEBUG, "Found Mapper : %d 0x%02X", map->pid, map->addr);

            pids.push_back(map->pid);
        }
    }
}

bool delete_mapper(core::common::MAPPER* mapper, int idx, int pid) {
    bool ret = false;
    core::common::MAPPER* map;
    for (map = mapper; map < mapper + idx; map++) {
        if (map->pid == pid) {
            map->pid = 0;
            ret = true;
        }
    }
    return ret;
}
void write_mapper(std::string filename, core::common::MAPPER* mapper) {
    FILE * f = fopen(filename.c_str(), "w");
    for (int i = 0; i < MAX_POOL; i++) {
        if (mapper[i].pid != 0) {
            fprintf(f, "%d %hd\n", mapper[i].pid, mapper[i].addr);
        }
    }
    fclose(f);
}

void read_mapper(std::string filename, core::common::MAPPER* mapper) {
    FILE * f = fopen(filename.c_str(), "r");

    for (int i = 0; i < MAX_POOL; i++) {
        fscanf(f, "%d %hd", &mapper[i].pid, &mapper[i].addr);
    }
    fclose(f);
}

int getTotalLine(string name) {
    FILE *fp;
    int line=0;
    char c;
    fp = fopen(name.c_str(), "r");
    while ((c = fgetc(fp)) != EOF) {
        if (c == '\n') {
            line++;
        }
    }
    fclose(fp);
    return(line);
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
	}
    // core::formatter::RSite rSite = {.status = false, .pid = 0 };
    createDataFile(RTU_DATA);
    createDataFile(CLIENT_DATA);

    if(test) {

        cout << sizeof(rtu_mapper_list) << "/" << sizeof(core::common::MAPPER)<< endl;

        // read_mapper(RTU_DATA, rtu_mapper_list);
        
        cout << getTotalLine(RTU_DATA) << endl;
        rtu_mapper_list[getTotalLine(RTU_DATA)] = add_mapper(123447, 1001);
        write_mapper(RTU_DATA, rtu_mapper_list);
        print_mapper(rtu_mapper_list);

        cout << getTotalLine(RTU_DATA) << endl;
        rtu_mapper_list[getTotalLine(RTU_DATA)] = add_mapper(123445, 1001);
        write_mapper(RTU_DATA, rtu_mapper_list);
        print_mapper(rtu_mapper_list);

        cout << getTotalLine(RTU_DATA) << endl;
        rtu_mapper_list[getTotalLine(RTU_DATA)] = add_mapper(123448, 1001);
        write_mapper(RTU_DATA, rtu_mapper_list);
        print_mapper(rtu_mapper_list);

        cout << getTotalLine(RTU_DATA) << endl;
        rtu_mapper_list[getTotalLine(RTU_DATA)] = add_mapper(123445, 1001);
        write_mapper(RTU_DATA, rtu_mapper_list);
        print_mapper(rtu_mapper_list);

        cout << getTotalLine(RTU_DATA) << endl;
        rtu_mapper_list[getTotalLine(RTU_DATA)] = add_mapper(123446, 1001);
        write_mapper(RTU_DATA, rtu_mapper_list);
        print_mapper(rtu_mapper_list);
        cout << "/////////////////////////////////////////" << endl;
        read_mapper(RTU_DATA, rtu_mapper_list);
        cout << "/////////////////////////////////////////" << endl;
        
        std::vector<pid_t> pids;
        search_mapper(rtu_mapper_list, pids, getTotalLine(RTU_DATA), 1001);
        for (auto it = pids.begin(); it!= pids.end(); it++) {
            cout << *it << endl;
        }

        // mapper_vector.push_back(add_mapper(123445, 1001));
        // mapper_vector.push_back(add_mapper(123446, 1002));
        // mapper_vector.push_back(add_mapper(123447, 1003));
        // mapper_vector.push_back(add_mapper(123448, 1004));

        // writeMapper(RTU_DATA, mapper_vector);
        // readMapper(RTU_DATA, mapper_vector);
        // print_vector(mapper_vector);

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
        char YYYYMMDD[9];
        char HHMMSS[7];
        time_t timer = time(NULL);
        // LocalTime
        struct tm* t = localtime(&timer);
        strftime(YYYYMMDD, sizeof(YYYYMMDD), "%Y%m%d", t);
        strftime(HHMMSS, sizeof(HHMMSS), "%H%M%S", t);
        cout << YYYYMMDD << endl;
        cout << HHMMSS << endl;
        // UTC
        gmtime_r(&timer, t);
        strftime(YYYYMMDD, sizeof(YYYYMMDD), "%Y%m%d", t);
        strftime(HHMMSS, sizeof(HHMMSS), "%H%M%S", t);
        cout << YYYYMMDD << endl;
        cout << HHMMSS << endl;
        //////////////////////////////////////////////////////////////
        string strAddr = find_rtu_addr(res.siteCode);
        cout << strAddr << endl;
        if (strAddr == NOT_FOUND) {
            strAddr = "0";
        }
        
        //////////////////////////////////////////////////////////////
        DATA ch[2];
        // u_short num으로 형변환 
        unsigned short num = stoi(strAddr);
        // 0x0001 으로 변환됨
        printf("hex : 0x%04X, %u \n", num, num);
        // u_short 을 char[] 변환, endian 변환
        ch[0]=(char)(num >> 8) | RTU_ADDRESS; // | 0x10 주의
        ch[1]=(char)(num & 0x00ff);
        printf("0x%02X, 0x%02X \n", ch[0], ch[1]);
        res.rtuAddr.setAddr(ch, sizeof(ch));
        printf("0x%04X, %u \n", res.rtuAddr.getAddr(), res.rtuAddr.getAddr());
        res.crc8.setCRC8(common::calcCRC((DATA*)&res, sizeof(res)));
        memcpy(sendbuf, (char*)&res, sizeof(res));
        common::print_hex(sendbuf, sizeof(res));
        //////////////////////////////////////////////////////////////
        // message queue 송수신 확인
        Mq mq;
        mq.open(RTU_MQ_NAME, getpid());

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
        bool rtu_running = false;
        bool cmd_running = false;
        server::ServerSocket rtu_server(rtu_port);
        server::ServerSocket cmd_server(cmd_port);
        syslog(LOG_DEBUG, "[Parent process] : listening.......");

        while(true) {
            common::sleep(100);
            server::ServerSocket new_sock;
            while((rtu_running = rtu_server.accept(new_sock)) || (cmd_running = cmd_server.accept(new_sock))) {
                u_short cmdAddr = 1;
                if (cmd_running) {
                    // Generate Client Address
                    if (!cmdAddrQueue.empty()) {
                        cmdAddr = cmdAddrQueue.top();
        syslog(LOG_DEBUG, "+----------+----------+--------+-------+");

                        cout << cmdAddr << endl;
                        syslog(LOG_DEBUG, "| 0x%02X |", cmdAddr);
                        cmdAddrQueue.pop();
                    }

                    if (cmdAddrQueue.empty()) {
                        for (u_short i = 0x01; i < 0xFF; i++) {
                            cmdAddrQueue.push(i);
                        }
                    }
                }
                
                pid = fork();

                if (pid == 0) {
                    syslog(LOG_DEBUG, "[Child process] %d << %d", getpid(), getppid());
                    rtu_server.close();
                    cmd_server.close();

                    start_child(new_sock, getpid(), cmdAddr);
                    
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

void start_child(server::ServerSocket newSock, int pid, u_short cmdAddr) {
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
        read_mapper(RTU_DATA, rtu_mapper_list);
        core::common::MAPPER cmd_mapper_list[MAX_POOL] = {0, };
        read_mapper(CLIENT_DATA, cmd_mapper_list);

        if (getTotalLine(RTU_DATA) > 0) {
            if (delete_mapper(rtu_mapper_list, getTotalLine(RTU_DATA), spid)) {
                write_mapper(RTU_DATA, rtu_mapper_list);
                print_mapper(rtu_mapper_list);

                file = "/dev/mqueue/rtu.";
                file.append(std::to_string(spid));
                unlink(file.c_str());
            }
        }
        if (getTotalLine(CLIENT_DATA) > 0) {
            if (delete_mapper(cmd_mapper_list, getTotalLine(CLIENT_DATA), spid)) {
                write_mapper(CLIENT_DATA, cmd_mapper_list);
                print_mapper(cmd_mapper_list);

                file = "/dev/mqueue/client.";
                file.append(std::to_string(spid));
                unlink(file.c_str());
            }
        }
        
        syslog(LOG_DEBUG, "Child Count : %ld", connected.size());
        cout << cmdAddrQueue.top() << endl;

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
