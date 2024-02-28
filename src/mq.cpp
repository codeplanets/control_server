
#include <unistd.h>
#include "mq.h"

const char* name_posix_mq = "/mq_";

namespace core {
    namespace system {
        Mq::Mq()
        : m_mq_fd((mqd_t)-1) {
            m_mq_attrib = {.mq_maxmsg = 10, .mq_msgsize = 1024};
        }

        Mq::~Mq() {
            close();
        }

        bool Mq::open(int pid) {

            // message queue 이름 설정 ("mq_" + pid)
            m_mqname = get_mq_name(name_posix_mq, pid);
            cout << "[" << getpid() << "] : " << m_mqname.c_str() << endl;

            // message queue 생성
            m_mq_fd = ::mq_open(m_mqname.c_str(), O_CREAT|O_EXCL|O_RDWR, S_IRUSR|S_IWUSR, &m_mq_attrib);
            if (m_mq_fd > 0) {
                cout << "[" << getpid() << "] : " << "Create Message Queue[" << m_mqname <<"] OK!" << endl;
            } else {
                // 이미 message queue가 생성되어 있으면 Open
                if (errno == EEXIST) {
                    cout << "[" << getpid() << "] : " << "Exist MQ, Open......" << endl;
                    m_mq_fd = ::mq_open(m_mqname.c_str(), O_RDWR);
                    if (m_mq_fd == (mqd_t)-1) {
                        cout << "[" << getpid() << "] : " << "Failed mq_open() : " << strerror(errno) << endl;
                        return false;
                    }
                    cout << "[" << getpid() << "] : " << "Open Message Queue[" << m_mqname <<"] OK!" << endl;
                } else {
                    cout << "[" << getpid() << "] : " << "Failed mq_open() : " << strerror(errno) << endl;
                    return false;
                }
            }
            return true;
        }
        
        void Mq::close(void) {
            // close message queue
            if (mq_close(m_mq_fd) < 0) {
                cout << "[" << getpid() << "] : " << "Failed mq_close() : " << strerror(errno) << endl;
            } else {
                cout << "[" << getpid() << "] : " << "mq_close()" << endl;
            }

            // remove message queue
            if (mq_unlink(m_mqname.c_str()) < 0) {
                cout << "[" << getpid() << "] : " << "Failed mq_unlink() : " << strerror(errno) << endl;
            } else {
                cout << "[" << getpid() << "] : " << "mq_unlink()" << endl;
            }
        }
        
        std::string Mq::get_mq_name(const char* name, int pid) {
            string mq_name;
            mq_name.append(name);
            mq_name.append(std::to_string(pid));
            cout << mq_name << endl;
            return mq_name.c_str();
        }

        bool Mq::send(const DATA* s, size_t len) {
            cout << "[" << getpid() << "] : " << "mq_send : " << s << endl;
            if (mq_send(m_mq_fd, s, len, 0) < 0) {
                cout << "[" << getpid() << "] : " << "Failed mq_send() : " << strerror(errno) << endl;
                return false;
            }
            return true;
        }

        bool Mq::recv(DATA* r, size_t len) {
            if (mq_receive(m_mq_fd, r, len, 0) < 0) {
                cout << "[" << getpid() << "] : " << "Failed mq_receive() : " << strerror(errno) << endl;
                return false;
            }
            cout << "[" << getpid() << "] : " << "mq_receive : " << r << endl;
            return true;
        }

        const Mq& Mq::operator << (const DATA* s) const {
            cout << "[" << getpid() << "] : " << "mq_send : " << s << endl;
            if (!mq_send(m_mq_fd, s, sizeof(s), 0)) {
                cout << "[" << getpid() << "] : " << "Failed mq_send() : " << strerror(errno) << endl;
                //throw SocketException(EC_WRITE_FAILURE, "Could not write to socket.");
            }
            
            return *this;
        }
        const Mq& Mq::operator >> (DATA* r) const {
            cout << "[" << getpid() << "] : " << "mq_receive : " << r << endl;
            if (!mq_receive(m_mq_fd, r, sizeof(r), 0)) {
                cout << "[" << getpid() << "] : " << "Failed mq_receive() : " << strerror(errno) << endl;
                // throw SocketException(EC_READ_FAILURE, "Could not read from socket.");
            }
            return *this;
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
