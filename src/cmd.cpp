#include <unistd.h>
#include <signal.h>

#include "cmd.h"
#include "packetizer.h"
#include "socketexception.h"

void cmd_timeout_handler(int sig) {
    if (sig == SIGALRM) {
        syslog(LOG_WARNING, "Timeout %d seconds", CMD_WAITING_SEC);
    }
    exit(EXIT_FAILURE);
}

namespace core {
    CMDclient::CMDclient(ServerSocket& sock)
    : Client(sock) {
        
    }

    CMDclient::~CMDclient() {
        mq.close();
    }

    void CMDclient::init(ClientInitReq &msg, u_short addr) {
        this->serverAddr = msg.fromAddr;
        this->cmdAddr.setAddr(addr, CLIENT_ADDRESS);
        this->m_addr = addr;

        syslog(LOG_DEBUG, "CMDclient::init : Server-0x%02X Client-0x%02X", this->serverAddr.getAddr(), this->cmdAddr.getAddr());
    }

    void CMDclient::setSiteCode(char* scode) {
        this->scode.setSiteCode(scode);

        syslog(LOG_DEBUG, "CMDclient::setSiteCode : %s", this->scode.getSiteCode());
    }

    int CMDclient::reqMessage(DATA* buf, DATA cmd) {
        // CLIENT_INIT_RES, COMMAND_RTU, COMMAND_CLIENT_ACK
        if (cmd == CLIENT_INIT_RES) {
            ClientInitRes msg;
            syslog(LOG_DEBUG, "CMDclient::reqMessage : ClientInitRes size = %ld", sizeof(msg));

            // Packetizer
            msg.fromAddr = this->serverAddr;
            msg.toAddr = this->cmdAddr;
            msg.clientAddr = this->cmdAddr;
            msg.crc8.setCRC8(common::calcCRC((DATA*)&msg, sizeof(msg)));
            msg.print();

            memcpy(buf, (char*)&msg, sizeof(msg));
            common::print_hex(buf, sizeof(msg));
            return sizeof(msg);

        } else if (cmd == COMMAND_RTU) {
            CommandRtu msg;
            syslog(LOG_DEBUG, "CMDclient::reqMessage : CommandRtu size = %ld", sizeof(msg));
            
            // Packetizer
            msg.fromAddr = this->rtuAddr;
            msg.toAddr = this->cmdAddr;
            msg.siteCode = this->scode;
            msg.dcCommand = this->dcCommand;
            msg.acCommand = this->acCommand;
            msg.crc8.setCRC8(common::calcCRC((DATA*)&msg, sizeof(msg)));
            msg.print();

            memcpy(buf, (char*)&msg, sizeof(msg));
            common::print_hex(buf, sizeof(msg));
            return sizeof(msg);

        } else if (cmd == COMMAND_CLIENT_ACK) {
            CommandClientAck msg;
            syslog(LOG_DEBUG, "CMDclient::reqMessage : CommandClientAck size = %ld", sizeof(msg));

            // Packetizer
            msg.fromAddr = this->rtuAddr;
            msg.toAddr = this->cmdAddr;
            msg.siteCode = this->scode;
            msg.dcCommand = this->dcCommand;
            msg.acCommand = this->acCommand;
            msg.result = this->cmdResult;
            msg.crc8.setCRC8(common::calcCRC((DATA*)&msg, sizeof(msg)));
            msg.print();

            memcpy(buf, (char*)&msg, sizeof(msg));
            common::print_hex(buf, sizeof(msg));
            return sizeof(msg);

        } else if (cmd == SETUP_INFO_ACK) {
            SetupInfoAck msg;
            syslog(LOG_DEBUG, "CMDclient::reqMessage : SetupInfoAck size = %ld", sizeof(msg));
            
            // Packetizer
            msg.fromAddr = this->serverAddr;
            msg.toAddr = this->cmdAddr;
            msg.action = this->action;
            msg.siteCode = this->scode;
            msg.result = this->actResult;
            msg.crc8.setCRC8(common::calcCRC((DATA*)&msg, sizeof(msg)));
            msg.print();

            memcpy(buf, (char*)&msg, sizeof(msg));
            common::print_hex(buf, sizeof(msg));
            return sizeof(msg);

        } else if (cmd == RTU_STATUS_RES) {
            
            int shm_fd;
            void* shm_ptr;
            system::SemLock status_lock(sem_rtu_status);
            size_t size = getcount_site();
            if (size == 0) {
                syslog(LOG_ERR, "No Site Code Found!");
            } else {
                syslog(LOG_INFO, "Site Code Found : %ld", size);
            }
            RtuStatus rtustatus[size];
            int bodysize = sizeof(rtustatus);

            status_lock.lock();
            shm_fd = shm_open(shm_rtu_status.c_str(), O_RDONLY, 0666);
            if (shm_fd == -1) {
                syslog(LOG_ERR, "shm_open error!");
                exit(EXIT_FAILURE);
            }
            // ftruncate(shm_fd, bodysize);
            shm_ptr = mmap(NULL, bodysize, PROT_READ, MAP_SHARED, shm_fd, 0);
            if (shm_ptr == MAP_FAILED) {
                syslog(LOG_ERR, "mmap error!");
                exit(EXIT_FAILURE);
            }
            memcpy((void*)rtustatus, shm_ptr, bodysize);
            munmap(shm_ptr, bodysize);  // close
            status_lock.unlock();

            // Packetizer
            RtuStatusResHead msgHead;
            RtuStatusResTail msgTail;
            syslog(LOG_DEBUG, "CMDclient::reqMessage : RtuStatusResHead size = %ld", sizeof(msgHead));

            msgHead.stx = STX;
            msgHead.cmd = RTU_STATUS_RES;
            msgHead.fromAddr = this->serverAddr;
            msgHead.toAddr.setAddr(0x0FFF, CLIENT_ADDRESS);
            msgHead.length = bodysize + sizeof(u_short);
            msgHead.count = size;
            msgHead.print();

            int headSize = sizeof(msgHead);
            int tailSize = sizeof(msgTail);
            int totSize = headSize + bodysize + tailSize;
            int idx = headSize;
            // head만큼 copy
            memcpy(buf, (char*)&msgHead, headSize);
            // head 끝부분 부터 bodysize만큼 copy
            memcpy(buf+idx, rtustatus, bodysize);
            idx += bodysize;
            memcpy(buf+idx, (char*)&msgTail, sizeof(msgTail));
            // head + body crc를 계산
            common::print_hex(buf, totSize);
            msgTail.crc8.setCRC8(common::calcCRC(buf, totSize));
            msgTail.print();
            memcpy(buf+idx, (char*)&msgTail, sizeof(msgTail));

            return totSize;

        } else {
            syslog(LOG_WARNING, "Unknown message type. : 0x%X", cmd);
        }

        return 0;
    }

    void CMDclient::setTimeout() {
        struct sigaction action;
        action.sa_handler = cmd_timeout_handler;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        sigaction(SIGALRM, &action, NULL);
        alarm(CMD_WAITING_SEC);
    }

    void CMDclient::run() {
        
        DATA sock_buf[MAX_RAW_BUFF] = {0x00,};
        DATA mq_buf[MQ_MSGSIZE] = {0x00,};
        DATA sendbuf[MAX_RAW_BUFF] = {0x00, };

        system::SemLock rtu_lock(sem_rtu_data);
        system::SemLock cmd_lock(sem_cmd_data);

        int len = reqMessage(sendbuf, CLIENT_INIT_RES);
        newSock.send(sendbuf, len);

        createMessageQueue(CLIENT_MQ_NAME);

        u_short addr = this->cmdAddr.getAddr();
        pid_t pid = getpid();
        if (addr > 0) {
            cmd_lock.lock();
            read_mapper(CLIENT_DATA, mapper_list);
            int line = getTotalLine(CLIENT_DATA);
            mapper_list[line] = add_mapper(pid, addr);
            write_mapper(CLIENT_DATA, mapper_list);
            print_mapper(mapper_list);
            cmd_lock.unlock();
        }

        while (true) {
            // Message Queue 수신 처리
            try {
                errno = 0;
                int rcvByte = mq.recv(mq_buf, sizeof(mq_buf));
                if (rcvByte > 0) {
                    if (mq_buf[0] == STX) {
                        if (mq_buf[1] == COMMAND_RTU_ACK) {  // Client
                            syslog(LOG_INFO, "MQ >> SVR : Command RTU Ack.");
                            common::print_hex(mq_buf, rcvByte);

                            CommandRtuAck msg;
                            assert(rcvByte == sizeof(msg));
                            memcpy((void*)&msg, mq_buf, rcvByte);
                            if (common::checkCRC((DATA*)&msg, rcvByte, msg.crc8.getCRC8()) == false) {
                                syslog(LOG_WARNING, "CRC Check Failed. : 0x%02X != 0x%02X", common::calcCRC((DATA*)&msg, rcvByte), msg.crc8.getCRC8());
                            }
                            msg.print();

                            this->scode = msg.siteCode;
                            this->dcCommand = msg.dcCommand;
                            this->acCommand = msg.acCommand;
                            this->cmdResult = msg.result;

                            setup_ack_value(cmdLog, this->cmdResult.getStrResult(), CONTROL_OK);
                            update_cmd_log(cmdLog);

                            syslog(LOG_INFO, "SVR >> CMD : Command Client Ack.");
                            len = reqMessage(sendbuf, COMMAND_CLIENT_ACK);
                            newSock.send(sendbuf, len);

                        } else if (mq_buf[1] == SETUP_INFO_ACK) {  // Client
                            syslog(LOG_INFO, "MQ >> CMD : Setup Info Ack.");
                            common::print_hex(mq_buf, rcvByte);
                            newSock.send(mq_buf, rcvByte);

                        } else if (mq_buf[1] == RTU_STATUS_RES) {  // All Client
                            syslog(LOG_INFO, "MQ >> CMD : RTU Status Res.");
                            common::print_hex(mq_buf, rcvByte);
                            newSock.send(mq_buf, rcvByte);

                        } else {
                            syslog(LOG_WARNING, "Unknown message type from mq. : 0x%X", mq_buf[1]);
                        }
                    }else {
                        syslog(LOG_WARNING, "Error Start of Text from mq. : 0x%X", mq_buf[0]);
                    }
                }
            } catch (exception& e) {
                syslog(LOG_CRIT, "[Error : %s:%d] Exception was caught : %s",__FILE__, __LINE__, e.what());
            }

            // Socket 수신 처리
            try {
                int msgSize = 0;
                int sendByte = 0;
                errno = 0;
                int rcvByte = newSock.peek(sock_buf, MAX_RAW_BUFF);
                if (rcvByte < 0) {
                    if (errno == EAGAIN) {
                        continue;   // 다시 mq > socket 순으로 데이터 수신
                    } else {
                        syslog(LOG_ERR, "CMDclient::run : %s", strerror(errno));
                        break;
                    }
                } else if (rcvByte == 0) {
                    continue;   // 다시 mq > socket 순으로 데이터 수신
                } else {
                    common::print_hex(sock_buf, rcvByte);
                    if (sock_buf[0] == STX) {
                        MsgHeader head;
                        memcpy((void*)&head, sock_buf, sizeof(head));
                        head.print();

                        msgSize = sizeof(head) + head.length + sizeof(MsgTail);
                        // DATA msg_buf[MAX_RAW_BUFF] = {0x00,};
                        // memcpy((void*)msg_buf, sock_buf, msgSize);
                        common::print_hex(sock_buf, msgSize);
                    } else {
                        msgSize = rcvByte;
                    }
                }

                rcvByte = newSock.recv(sock_buf, msgSize);
                if (sock_buf[0] == STX) {
                    if (sock_buf[1] == COMMAND_CLIENT) {
                        syslog(LOG_INFO, "CMD >> SVR : Command Client.");
                        alarm(CMD_WAITING_SEC);

                        CommandClient msg;
                        assert(rcvByte == sizeof(msg));
                        memcpy((void*)&msg, sock_buf, rcvByte);
                        if (common::checkCRC((DATA*)&msg, rcvByte, msg.crc8.getCRC8()) == false) {
                            syslog(LOG_WARNING, "CRC Check Failed. : 0x%02X != 0x%02X", common::calcCRC((DATA*)&msg, rcvByte), msg.crc8.getCRC8());
                        }
                        msg.print();

                        this->scode = msg.siteCode;
                        this->dcCommand = msg.dcCommand;
                        this->acCommand = msg.acCommand;
                        
                        setup_init_value(cmdLog);
                        insert_cmd_log(cmdLog);

                        string addr = find_rtu_addr(this->scode);
                        if (addr != NOT_FOUND) {
                            this->rtuAddr.setAddr(addr, RTU_ADDRESS);
                            // TODO : 해당 Address의 RTU MQ에 send 해야함.
                            sendByte = reqMessage(sendbuf, COMMAND_RTU);
                            Mq rtu_mq;
                            pid_t pid = 0;
                            std::vector<pid_t> pids;
                            // data에서 pid를 read.
                            rtu_lock.lock();
                            read_mapper(RTU_DATA, mapper_list);
                            print_mapper(mapper_list);
                            int line = getTotalLine(RTU_DATA);
                            rtu_lock.unlock();
                            search_mapper(mapper_list, pids, line, this->rtuAddr.getAddr());
                            cout << "RTU PIDs : ";
                            for (auto it = pids.begin(); it!= pids.end(); it++) {
                                cout << *it << " ";
                                pid = *it;
                                if (pid != 0) {
                                    syslog(LOG_INFO, "SVR >> MQ : Command RTU. : %d", pid);
                                    rtu_mq.open(RTU_MQ_NAME, pid);
                                    rtu_mq.send(sendbuf, sendByte);
                                    rtu_mq.close();
                                }
                            }
                            cout << endl;

                            if (pid == 0) {
                                syslog(LOG_WARNING, "Not Connected RTU. : %s", this->scode.getSiteCode());
                                setup_ack_value(cmdLog, "N", NOT_CONNECT);
                            }

                        } else {
                            syslog(LOG_WARNING, "Unknown Site Code from client. : %s", this->scode.getSiteCode());
                            setup_ack_value(cmdLog, "N", SITE_NOT_FOUND);
                        }

                    } else if (sock_buf[1] == SETUP_INFO) {
                        syslog(LOG_INFO, "CMD >> SVR : Setup Info.");
                        alarm(CMD_WAITING_SEC);
                        
                        SetupInfo msg;
                        assert(rcvByte == sizeof(msg));
                        memcpy((void*)&msg, sock_buf, rcvByte);
                        if (common::checkCRC((DATA*)&msg, rcvByte, msg.crc8.getCRC8()) == false) {
                            syslog(LOG_WARNING, "CRC Check Failed. : 0x%02X != 0x%02X", common::calcCRC((DATA*)&msg, rcvByte), msg.crc8.getCRC8());
                        }
                        msg.print();
                        
                        // 값들을 저장
                        bool shutdown = false;
                        this->serverAddr = msg.fromAddr;
                        this->rtuAddr = msg.toAddr;
                        this->action = msg.action;
                        this->scode = msg.siteCode;
                        DATA result = ACTION_RESULT_FAIL;
                        
                        // Action I, U, D
                        char action = this->action.getAction();
                        if (action == 'Q') {
                            syslog(LOG_INFO, "Setup Info Action Q!");
                            shutdown = true;
                            result = ACTION_RESULT_OK;
                        }

                        this->actResult.setResult(result);
                        syslog(LOG_INFO, "SVR >> CMD : Setup Info Action Ack %c!", this->action.getAction());
                        sendByte = reqMessage(sendbuf, SETUP_INFO_ACK);
                        newSock.send(sendbuf, sendByte);

                        if (shutdown) {
                            syslog(LOG_INFO, "Server Shutdown.");
                            // TODO: kill process
                            ::system("./pkill.sh");
                            break;
                        }

                        Mq rtu_mq;
                        pid_t pid = 0;
                        std::vector<pid_t> pids;
                        // data에서 pid를 read.
                        rtu_lock.lock();
                        read_mapper(RTU_DATA, mapper_list);
                        int line = getTotalLine(RTU_DATA);
                        rtu_lock.unlock();
                        search_mapper(mapper_list, pids, line, this->rtuAddr.getAddr());
                        cout << "RTU PIDs : ";
                        for (auto it = pids.begin(); it!= pids.end(); it++) {
                            cout << *it << " ";
                            pid = *it;
                            if (pid != 0) {
                                syslog(LOG_INFO, "SVR >> MQ : Setup Info Ack. : %d", pid);
                                rtu_mq.open(RTU_MQ_NAME, pid);
                                rtu_mq.send(sendbuf, sendByte);
                                rtu_mq.close();
                            }
                        }
                        cout << endl;

                        if (pid == 0) {
                            syslog(LOG_WARNING, "Not Connected RTU. : %s", this->scode.getSiteCode());
                            syslog(LOG_INFO, "SVR >> CMD : Setup Info Ack %c!", this->action.getAction());
                            newSock.send(sendbuf, sendByte);
                        }

                        

                    } else if (sock_buf[1] == RTU_STATUS_REQ) {
                        syslog(LOG_INFO, "CMD >> SVR : RTU Status Req.");
                        alarm(CMD_WAITING_SEC);

                        RtuStatusReq msg;
                        assert(rcvByte == sizeof(msg));
                        memcpy((void*)&msg, sock_buf, rcvByte);
                        if (common::checkCRC((DATA*)&msg, rcvByte, msg.crc8.getCRC8()) == false) {
                            syslog(LOG_WARNING, "CRC Check Failed. : 0x%02X != 0x%02X", common::calcCRC((DATA*)&msg, rcvByte), msg.crc8.getCRC8());
                        }
                        msg.print();

                        this->serverAddr = msg.fromAddr;    // * 확인 *
                        this->cmdAddr = msg.toAddr;         // * 확인 *

                        // TODO : 모든 Address의 Client MQ에 send 해야함.
                        sendByte = reqMessage(sendbuf, RTU_STATUS_RES);
                        Mq cmd_mq;
                        std::vector<pid_t> pids;
                        // data에서 pid를 read.
                        core::common::MAPPER cmd_mapper_list[MAX_POOL] = {0, };
                        cmd_lock.lock();
                        read_mapper(CLIENT_DATA, cmd_mapper_list);
                        int line = getTotalLine(CLIENT_DATA);
                        cmd_lock.unlock();
                        core::common::MAPPER* map;
                        for (map = cmd_mapper_list; map < cmd_mapper_list + line; map++) {
                            if (map->pid != 0) {
                                if (cmd_mq.open(CLIENT_MQ_NAME, map->pid)) {
                                    syslog(LOG_INFO, "SVR >> MQ : RTU Status Res. : %d", map->pid);
                                    cmd_mq.send(sendbuf, sendByte);
                                    cmd_mq.close();
                                } else {
                                    syslog(LOG_WARNING, "Failed to open Client MQ. : %d", map->pid);                                    
                                }
                            }
                        }
                        
                    } else {
                        syslog(LOG_WARNING, "Unknown message type from socket.");
                    }
                } else {
                    syslog(LOG_WARNING, "Error Start of Text from socket.");
                }
            } catch (SocketException& se) {
                syslog(LOG_CRIT, "[Error : %s:%d] Exception was caught : [%d] %s",__FILE__, __LINE__, se.code(), se.description().c_str());
            }
            common::sleep(100);

        }
    }
    
    bool CMDclient::setup_init_value(CmdLog &log) {
        char YYYYMMDD[9];
        char HHMMSS[7];
        time_t timer = time(NULL);
        struct tm* t = localtime(&timer);
        strftime(YYYYMMDD, sizeof(YYYYMMDD), "%Y%m%d", t);
        strftime(HHMMSS, sizeof(HHMMSS), "%H%M%S", t);
        log.siteCode = this->scode.getSiteCode();
        log.date = YYYYMMDD;
        log.time = HHMMSS;
        log.ackDate = "";
        log.ackTime = "";
        log.ackResult = "N";
        log.resultCode = NOT_ACK;

        return true;
    }

    bool CMDclient::setup_ack_value(CmdLog &log, std::string result, int code) {
        char YYYYMMDD[9];
        char HHMMSS[7];
        time_t timer = time(NULL);
        struct tm* t = localtime(&timer);
        strftime(YYYYMMDD, sizeof(YYYYMMDD), "%Y%m%d", t);
        strftime(HHMMSS, sizeof(HHMMSS), "%H%M%S", t);
        log.ack = true;
        log.ackDate = YYYYMMDD;
        log.ackTime = HHMMSS;
        log.ackResult = result;
        log.resultCode = code;

        return true;
    }

    bool CMDclient::insert_cmd_log(CmdLog &log) {
        Database db;
        // ECODE ecode = db.db_init("localhost", 3306, "rcontrol", "rcontrol2024", "RControl");
        ECODE ecode = db.db_init();
        if (ecode!= EC_SUCCESS) {
            syslog(LOG_ERR, "DB Connection Error!");
            return false;
        }

        string insert = "INSERT INTO RHistory(SiteCode, ControlDate, ControlTime, AckResult, ResultCode)";
        insert += " VALUES ('";
        insert += log.siteCode;
        insert += "', '";
        insert += log.date;
        insert += "', '";
        insert += log.time;
        insert += "', '";
        insert += log.ackResult;
        insert += "', ";
        insert += to_string(log.resultCode);
        insert += ");";
        syslog(LOG_DEBUG, "Query : %s", insert.c_str());

        ecode = db.db_query(insert.c_str());
        if (ecode != EC_SUCCESS) {
            syslog(LOG_ERR, "DB Insert Query Error!");
            return false;
        }
        return true;
    }

    bool CMDclient::update_cmd_log(CmdLog &log) {
        if (!log.ack) {
            return false;
        }
        
        Database db;
        // ECODE ecode = db.db_init("localhost", 3306, "rcontrol", "rcontrol2024", "RControl");
        ECODE ecode = db.db_init();
        if (ecode!= EC_SUCCESS) {
            syslog(LOG_ERR, "DB Connection Error!");
            return false;
        }
        
        string insert = "UPDATE RHistory SET ";
        insert += " AckDate = '";
        insert += log.ackDate;
        insert += "', AckTime = '";
        insert += log.ackTime;
        insert += "', AckResult = '";
        insert += log.ackResult;
        insert += "', ResultCode = ";
        insert += to_string(log.resultCode);
        insert += " WHERE SiteCode = '";
        insert += log.siteCode;
        insert += "' AND ControlDate = '";
        insert += log.date;
        insert += "' AND ControlTime = '";
        insert += log.time;
        insert += "';";
        syslog(LOG_DEBUG, "Query : %s", insert.c_str());

        ecode = db.db_query(insert.c_str());
        if (ecode != EC_SUCCESS) {
            syslog(LOG_ERR, "DB Update Query Error!");
            return false;
        }
        return true;
    }

}
