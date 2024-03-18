
#include <unistd.h>
#include "mq.h"

const string name_posix_mq = "/server.";

namespace core {
    namespace system {
        Mq::Mq()
        : m_mq_fd((mqd_t)-1) {
            m_mq_attrib = {.mq_maxmsg = 10, .mq_msgsize = 1024};
        }

        Mq::~Mq() {
            close();
        }

        bool Mq::open(const string name, int pid) {

            // message queue 이름 설정 ("mq_" + pid)
            m_mqname = make_mq_name(name, pid);
            syslog(LOG_INFO, "Message Queue name : %s", m_mqname.c_str());

            // message queue 생성
            m_mq_fd = ::mq_open(m_mqname.c_str(), O_CREAT|O_EXCL|O_RDWR, S_IRUSR|S_IWUSR, &m_mq_attrib);
            if (m_mq_fd > 0) {
                syslog(LOG_DEBUG, "Create Message Queue[%s] OK!", m_mqname.c_str());
            } else {
                // 이미 message queue가 생성되어 있으면 Open
                if (errno == EEXIST) {
                    syslog(LOG_WARNING, "Exist MQ, Open......");
                    m_mq_fd = ::mq_open(m_mqname.c_str(), O_RDWR);
                    if (m_mq_fd == (mqd_t)-1) {
                        syslog(LOG_ERR, "Failed mq_open() : %s", strerror(errno));
                        return false;
                    }
                    syslog(LOG_DEBUG, "Open Message Queue[%s] OK!", m_mqname.c_str());
                } else {
                    syslog(LOG_ERR, "Failed mq_open() : %s", strerror(errno));
                    return false;
                }
            }
            return true;
        }
        
        void Mq::close(void) {
            // close message queue
            if (m_mq_fd != (mqd_t)-1) {
                if (mq_close(m_mq_fd) < 0) {
                    // syslog(LOG_ERR, "Failed mq_close() : %s", strerror(errno));
                } else {
                    syslog(LOG_DEBUG, "mq_close()");
                }

                // remove message queue
                // if (mq_unlink(m_mqname.c_str()) < 0) {
                //     syslog(LOG_ERR, "Failed mq_unlink() : %s", strerror(errno));
                // } else {
                //     syslog(LOG_DEBUG, "mq_unlink()");
                // }
            }
        }
        
        std::string Mq::make_mq_name(const string name, int pid) {
            string mq_name;
            mq_name.append(name);
            mq_name.append(std::to_string(pid));
            return mq_name.c_str();
        }

        std::string Mq::get_mq_name() {
            return this->m_mqname;
        }

        bool Mq::send(DATA* s, size_t len) {
            syslog(LOG_DEBUG, "mq_send : %ld Bytes", len);
            common::print_hex(s, len);

            if (mq_send(m_mq_fd, (char*)s, len, 0) < 0) {
                syslog(LOG_ERR, "Failed mq_send() : %s", strerror(errno));
                return false;
            }
            return true;
        }

        bool Mq::recv(DATA* r, size_t len) {
            mq_getattr(m_mq_fd, &m_mq_attrib);
            if (m_mq_attrib.mq_curmsgs == 0) {
                return false;
            }
            
            int rcvByte = mq_receive(m_mq_fd, (char*)r, len, 0);
            if (rcvByte < 0) {
                syslog(LOG_ERR, "Failed mq_receive() : %s", strerror(errno));
                return false;
            }
            syslog(LOG_DEBUG, "mq_receive : %d Bytes", rcvByte);
            common::print_hex(r, rcvByte);

            return true;
        }
    }
}


// void chk_rt(int sig, siginfo_t *si, void *data) {
//     cout << "[SIGRT] si_code, si_band, si_fd, si_value :";
//     cout << si->si_code << ", ";
//     cout << si->si_band << ", ";
//     cout << si->si_fd << ", ";
//     cout << si->si_value.sival_int << endl;
// }

// bool init_signal(mqd_t mq_fd) {
//     // message queue 시그널 핸들러 설치
//     struct sigevent sigev_noti;
//     struct sigaction sa_rt;
//     struct mq_attr mq_at;
//     sa_rt.sa_sigaction = chk_rt;
//     sigemptyset(&sa_rt.sa_mask);
//     sa_rt.sa_flags = SA_SIGINFO|SA_RESTART;
//     sigaction(SIGRTMIN, &sa_rt, NULL);

//     // 시그널 통지 이벤트 구조체 작성
//     memset(&sigev_noti, 0, sizeof(struct sigevent));
//     sigev_noti.sigev_notify = SIGEV_SIGNAL;

//     // notification via signal delivery
//     sigev_noti.sigev_signo = SIGRTMIN;
//     sigev_noti.sigev_value.sival_int = 0x64; // 전달될 데이터 (int형)

//     return true;

//     // mq_getattr(mq_fd, &mq_at);
//     // if (mq_at.mq_curmsgs == 0) {
//     //     if (mq_notify(mq_fd, &sigev_noti) == -1) {
//     //         cout << "[" << getpid() << "] : " << "Failed mq_notify() : " << strerror(errno) << endl;
//     //         return false;
//     //     }
//     // }
//     // cout << "[" << getpid() << "] : " << "Waiting Signal....." << endl;
//     // char buf[1024] = {0x00,};
//     // cout << recv(buf, sizeof(buf)) << endl;
// }
