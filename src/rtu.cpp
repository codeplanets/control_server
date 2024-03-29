#include <unistd.h>
#include <signal.h>

#include "rtu.h"
#include "socketexception.h"

void rtu_timeout_handler(int sig) {
    if (sig == SIGALRM) {
        syslog(LOG_WARNING, "Timeout %d seconds : ", WAITING_SEC);
    }
    exit(EXIT_FAILURE);
}

namespace core {
    RTUclient::RTUclient(ServerSocket& sock)
    : Client(sock) {
        
    }

    RTUclient::~RTUclient() {
        mq.close();
    }

    int RTUclient::reqMessage(DATA* buf, DATA cmd) {
        // INIT_RES, HEART_BEAT_ACK, COMMAND_RTU
        if (cmd == INIT_RES) {
            InitRes msg;
            syslog(LOG_DEBUG, "RTUclient::reqMessage : InitRes size = %ld", sizeof(msg));

            string addr = find_rtu_addr(this->scode);
            if (addr != NOT_FOUND) {
                this->rtuAddr.setAddr(addr, RTU_ADDRESS);
            } else {
                syslog(LOG_WARNING, "Unknown Site Code from RTU. : %s", this->scode.getSiteCode());
                this->rtuAddr.setAddr(0x0000);
            }

            // Packetizer
            msg.fromAddr = this->rtuAddr;
            msg.toAddr = this->serverAddr;
            msg.siteCode = this->scode;
            msg.rtuAddr = this->rtuAddr;
            msg.crc8.setCRC8(common::calcCRC((DATA*)&msg, sizeof(msg)));
            msg.print();

            memcpy(buf, (char*)&msg, sizeof(msg));
            common::print_hex(buf, sizeof(msg));
            return sizeof(msg);

        } else if (cmd == HEART_BEAT_ACK) {
            HeartBeatAck msg;
            syslog(LOG_DEBUG, "RTUclient::reqMessage : Heartbeat Ack size = %ld", sizeof(msg));
            
            msg.fromAddr = this->rtuAddr;
            msg.toAddr = this->serverAddr;

            msg.crc8.setCRC8(common::calcCRC((DATA*)&msg, sizeof(msg)));
            msg.print();

            memcpy(buf, (char*)&msg, sizeof(msg));
            common::print_hex(buf, sizeof(msg));
            return sizeof(msg);

        } else if (cmd == COMMAND_RTU) {
            CommandRtu msg;
            syslog(LOG_DEBUG, "RTUclient::reqMessage : Command RTU size = %ld", sizeof(msg));

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

        } else {
            syslog(LOG_WARNING, "Unknown message type. : 0x%X", cmd);
        }
        return 0;
    }
    
    void RTUclient::setTimeout() {
        struct sigaction action;
        action.sa_handler = rtu_timeout_handler;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        sigaction(SIGALRM, &action, NULL);
        alarm(WAITING_SEC);
    }

    void RTUclient::setStatus(u_short sid, DATA status) {
        /////////////////////////////////////////////////
        // Child
        system::SemLock status_lock(sem_rtu_status);
        size_t size = getcount_site();
        if (size == 0) {
            syslog(LOG_ERR, "[Error : %s:%d] Failed : Not found Site Code! : %s",__FILE__, __LINE__, strerror(errno));
            return;
        } else {
            syslog(LOG_DEBUG, "Site Code Found : %ld", size);
        }
        RtuStatus rtustatus[size];
        status_lock.lock();
        shm_fd = shm_open(shm_rtu_status.c_str(), O_RDWR, 0666);
        if (shm_fd == -1) {
            syslog(LOG_ERR, "[Error : %s:%d] Failed : shm_open() error! : %s",__FILE__, __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }
        // ftruncate(shm_fd, sizeof(rtustatus));
        shm_ptr = mmap(NULL, sizeof(rtustatus), PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (shm_ptr == MAP_FAILED) {
            syslog(LOG_ERR, "[Error : %s:%d] Failed : mmap() error! : %s",__FILE__, __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }

        memcpy((void*)rtustatus, shm_ptr, sizeof(rtustatus));
        for (size_t i = 0; i < size; i++) {
            RtuStatus st = rtustatus[i];
            if (sid == st.siteid.getAddr()) {
                rtustatus[i].status.setStatus(status);
                if (test) {
                    printf("%02X-0x%0X>0x%0X\n", st.siteid.getAddr() ,st.status.getStatus(), status);
                }
            }
        }
        memcpy((void*)shm_ptr, rtustatus, sizeof(rtustatus));
        munmap(shm_ptr, sizeof(rtustatus));  // close
        status_lock.unlock();
    }

    void RTUclient::run() {
        
        DATA sock_buf[MAX_RAW_BUFF] = {0x00,};
        DATA mq_buf[MQ_MSGSIZE] = {0x00,};
        DATA sendbuf[MAX_RAW_BUFF] = {0x00, };

        system::SemLock rtu_lock(sem_rtu_data);
        system::SemLock cmd_lock(sem_cmd_data);

        createMessageQueue(RTU_MQ_NAME);

        while (true) {
            // Command Client Message Queue
            try {
                errno = 0;
                if (mq.open(RTU_MQ_NAME, getpid())) {
                    int rcvByte = mq.recv(mq_buf, sizeof(mq_buf));
                    mq.close();
                    int sendByte = 0;
                    if (rcvByte > 0) {
                        if (mq_buf[0] == STX) {
                            if (mq_buf[1] == COMMAND_RTU) {  // Client
                                syslog(LOG_INFO, "MQ >> RTU : Command RTU.");
                                common::print_hex(mq_buf, rcvByte);

                                newSock.send(mq_buf, rcvByte);
                            } else if (mq_buf[1] == SETUP_INFO_ACK) {  // Client
                                syslog(LOG_INFO, "MQ >> SVR : Setup Info Ack.");
                                
                                SetupInfoAck msg;
                                assert(rcvByte == sizeof(msg));
                                memcpy((void*)&msg, mq_buf, rcvByte);
                                if (common::checkCRC((DATA*)&msg, rcvByte, msg.crc8.getCRC8()) == false) {
                                    syslog(LOG_WARNING, "CRC Check Failed. : 0x%02X != 0x%02X", common::calcCRC((DATA*)&msg, rcvByte), msg.crc8.getCRC8());
                                }
                                msg.print();

                                // 값들을 저장
                                SiteCode scode_old = this->scode;
                                this->action = msg.action;
                                this->scode = msg.siteCode;
                                DATA result = ACTION_RESULT_OK;

                                char action = this->action.getAction();
                                if (action != 'Q') {
                                    if (isSiteCodeAvailable() == false) {
                                        result = ACTION_RESULT_FAIL;
                                        this->scode = scode_old;    // restore sitecode
                                    } else {
                                        result = ACTION_RESULT_OK;
                                    }
                                } else {
                                    syslog(LOG_INFO, "Setup Info Action Q!");
                                    return;
                                }
                                this->actResult.setResult(result);
                                sendByte = reqMessage(sendbuf, SETUP_INFO_ACK);

                                // Client MQ 에 데이터를 전송
                                Mq cmd_mq;
                                pid_t pid = 0;
                                std::vector<pid_t> pids;
                                cmd_lock.lock();
                                read_mapper(CLIENT_DATA, cmd_mapper_list);
                                int line = getTotalLine(CLIENT_DATA);
                                cmd_lock.unlock();

                                syslog(LOG_DEBUG, "Client Address : 0x%02X Searching...", msg.toAddr.getAddr());
                                search_mapper(cmd_mapper_list, pids, line, msg.toAddr.getAddr());

                                for (auto it = pids.begin(); it!= pids.end(); it++) {
                                    pid = *it;
                                    if (pid != 0) {
                                        if (cmd_mq.open(CLIENT_MQ_NAME, pid)) {
                                            syslog(LOG_INFO, "SVR >> MQ : Setup Info Ack. : %d", pid);
                                            cmd_mq.send(sendbuf, sendByte);
                                            cmd_mq.close();
                                        } else {
                                            syslog(LOG_WARNING, "Failed to open Client MQ. : %d", pid);
                                        }
                                    }
                                }
                            } else {
                                syslog(LOG_WARNING, "Unknown message type from mq. : 0x%X", mq_buf[1]);
                            }
                        }else {
                            syslog(LOG_WARNING, "Error Start of Text from mq. : 0x%X", mq_buf[0]);
                        }
                    }
                } else {
                    syslog(LOG_WARNING, "Failed to open RTU MQ. : %d", getpid());
                }
            } catch (exception& e) {
                syslog(LOG_CRIT, "[Error : %s:%d] Exception was caught : %s",__FILE__, __LINE__, e.what());
            }

            try {
                int msgSize = 0;
                int sendByte = 0;
                errno = 0;
                int rcvByte = newSock.peek(sock_buf, MAX_RAW_BUFF);
                if (rcvByte < 0) {
                    if (errno == EAGAIN) {
                        common::sleep(100);
                        continue;
                    } else {
                        syslog(LOG_ERR, "[Error : %s:%d] Failed : peek() error! : %s",__FILE__, __LINE__, strerror(errno));
                        break;
                    }
                } else if (rcvByte == 0) {
                    common::sleep(100);
                    continue;
                } else {
                    common::print_hex(sock_buf, rcvByte);
                    if (sock_buf[0] == STX) {
                        MsgHeader head;
                        memcpy((void*)&head, sock_buf, sizeof(head));
                        head.print();

                        msgSize = sizeof(head) + head.length + sizeof(MsgTail);
                        common::print_hex(sock_buf, msgSize);
                    } else {
                        msgSize = rcvByte;
                    }
                }

                rcvByte = newSock.recv(sock_buf, msgSize);
                if (sock_buf[0] == STX) {
                    if (sock_buf[1] == INIT_REQ) {  // RTUs
                        syslog(LOG_INFO, "RTU >> SVR : RTU Init Req.");
                        alarm(WAITING_SEC);

                        InitReq msg;
                        assert(rcvByte == sizeof(msg));
                        memcpy((void*)&msg, sock_buf, rcvByte);
                        if (common::checkCRC((DATA*)&msg, rcvByte, msg.crc8.getCRC8()) == false) {
                            syslog(LOG_WARNING, "CRC Check Failed. : 0x%02X != 0x%02X", common::calcCRC((DATA*)&msg, rcvByte), msg.crc8.getCRC8());
                        }
                        msg.print();

                        this->rtuAddr = msg.fromAddr;
                        this->serverAddr = msg.toAddr;
                        this->scode = msg.siteCode;

                        syslog(LOG_INFO, "SVR >> RTU : RTU Init Res.");
                        if (isSiteCodeAvailable() == false) {
                            sendByte = reqMessage(sendbuf, INIT_RES);
                            newSock.send(sendbuf, sendByte);
                            common::sleep(5000);
                            return;
                        }

                        sendByte = reqMessage(sendbuf, INIT_RES);
                        newSock.send(sendbuf, sendByte);

                        u_short addr = this->rtuAddr.getAddr();
                        pid_t rtu_pid = getpid();
                        if (addr > 0) {
                            rtu_lock.lock();
                            read_mapper(RTU_DATA, mapper_list);
                            int line = getTotalLine(RTU_DATA);
                            pid_t pid = 0;
                            search_mapper(mapper_list, pid, line, addr);
                            if (pid == 0) {
                                mapper_list[line] = add_mapper(rtu_pid, addr);
                                write_mapper(RTU_DATA, mapper_list);
                            }
                            print_mapper(mapper_list);
                            // std::set<std::pair<pid_t, u_short>> pidaddr_rtu;
                            // read_pair(RTU_DATA, pidaddr_rtu);
                            // pidaddr_rtu.insert(add_pair(rtu_pid, addr));
                            // write_pair(RTU_DATA, pidaddr_rtu);
                            // print_pair(pidaddr_rtu);
                            rtu_lock.unlock();
                        }

                        u_short sid = this->rtuAddr.getAddr();
                        setStatus(sid, STATUS_CONNECTED);

                    } else if (sock_buf[1] == HEART_BEAT) {  // RTU
                        syslog(LOG_DEBUG, "RTU >> SVR : Heartbeat.");
                        alarm(WAITING_SEC);
                        
                        HeartBeat msg;
                        assert(rcvByte == sizeof(msg));
                        memcpy((void*)&msg, sock_buf, rcvByte);
                        if (common::checkCRC((DATA*)&msg, rcvByte, msg.crc8.getCRC8()) == false) {
                            syslog(LOG_WARNING, "CRC Check Failed. : 0x%02X != 0x%02X", common::calcCRC((DATA*)&msg, rcvByte), msg.crc8.getCRC8());
                        }
                        msg.print();

                        syslog(LOG_DEBUG, "SVR >> RTU : Heartbeat Ack.");
                        sendByte = reqMessage(sendbuf, HEART_BEAT_ACK);
                        newSock.send(sendbuf, sendByte);

                    } else if (sock_buf[1] == COMMAND_RTU_ACK) {  // Client
                        syslog(LOG_INFO, "RTU >> SVR : Command RTU Ack.");
                        CommandRtuAck msg;
                        assert(rcvByte == sizeof(msg));
                        memcpy((void*)&msg, sock_buf, rcvByte);
                        if (common::checkCRC((DATA*)&msg, rcvByte, msg.crc8.getCRC8()) == false) {
                            syslog(LOG_WARNING, "CRC Check Failed. : 0x%02X != 0x%02X", common::calcCRC((DATA*)&msg, rcvByte), msg.crc8.getCRC8());
                        }
                        msg.print();

                        // Client MQ 에 데이터를 전송
                        Mq cmd_mq;
                        pid_t pid = 0;
                        std::vector<pid_t> pids;
                        cmd_lock.lock();
                        // data에서 status read.
                        read_mapper(CLIENT_DATA, cmd_mapper_list);
                        int line = getTotalLine(CLIENT_DATA);
                        cmd_lock.unlock();

                        syslog(LOG_DEBUG, "Client Address : 0x%02X Searching...", msg.toAddr.getAddr());
                        search_mapper(cmd_mapper_list, pids, line, msg.toAddr.getAddr());
                        for (auto it = pids.begin(); it!= pids.end(); it++) {
                            pid = *it;
                            if (pid != 0) {
                                if (cmd_mq.open(CLIENT_MQ_NAME, pid)) {
                                    syslog(LOG_INFO, "SVR >> MQ : Command RTU Ack. : %d", pid);
                                    cmd_mq.send(sock_buf, sizeof(msg));
                                    cmd_mq.close();
                                } else {
                                    syslog(LOG_WARNING, "Failed to open Client MQ. : %d", pid);
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
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    std::pair<pid_t, u_short> RTUclient::add_pair(pid_t pid, u_short addr) {
        return std::make_pair(pid, addr);
    }

    void RTUclient::print_pair(std::set<std::pair<pid_t, u_short>> s) {
        for (std::pair<pid_t, u_short> p : s) {
            syslog(LOG_DEBUG, "print_pair() : %d 0x%02X", p.first, p.second);
        }
    }

    bool RTUclient::search_pair(std::set<std::pair<pid_t, u_short>> s, u_short addr, pid_t &pid) {
        bool ret = false;
        for (std::pair<pid_t, u_short> p : s) {
            if (p.second == addr) {
                pid = p.first;
                syslog(LOG_DEBUG, "search_pair() : %d 0x%02X", p.first, p.second);
                ret = true;
                break;
            }
        }
        return ret;
    }

    bool RTUclient::search_pair(std::set<std::pair<pid_t, u_short>> s, pid_t pid, u_short &addr) {
        bool ret = false;
        for (std::pair<pid_t, u_short> p : s) {
            if (p.first == pid) {
                addr = p.second;
                syslog(LOG_DEBUG, "search_pair() : %d 0x%02X", p.first, p.second);
                ret = true;
                break;
            }
        }
        return ret;
    }

    bool RTUclient::search_pair(std::set<std::pair<pid_t, u_short>> s, u_short addr, std::vector<pid_t> &pids) {
        bool ret = false;
        for (std::pair<pid_t, u_short> p : s) {
            if (p.second == addr) {
                pids.push_back(p.first);
                syslog(LOG_DEBUG, "search_pair() : %d 0x%02X", p.first, p.second);
                ret = true;
            }
        }
        return ret;
    }

    bool RTUclient::delete_pair(std::set<std::pair<pid_t, u_short>> &s, int pid) {
        bool ret = false;
        for (std::pair<pid_t, u_short> p : s) {
            if (p.first == pid) {
                s.erase(p);
                syslog(LOG_DEBUG, "delete_pair() : %d", pid);
                ret = true;
            }
        }
        return ret;
    }

    void RTUclient::write_pair(std::string filename, std::set<std::pair<pid_t, u_short>> s) {
        FILE * f = fopen(filename.c_str(), "w");
        for (std::pair<pid_t, u_short> p : s) {
            fprintf(f, "%d %hd\n", p.first, p.second);
        }
        syslog(LOG_DEBUG, "write_pair()");
        fclose(f);
    }

    void RTUclient::read_pair(std::string filename, std::set<std::pair<pid_t, u_short>> &s) {
        FILE * f = fopen(filename.c_str(), "r");
        char buf[1024];
        while (fgets(buf, 1024, f)!= NULL) {
            pid_t pid;
            u_short addr;
            sscanf(buf, "%d %hd", &pid, &addr);
            s.insert(add_pair(pid, addr));
        }
        fclose(f);
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////
}
