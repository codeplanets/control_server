
#include <unistd.h>
#include "mq.h"

const std::string name_posix_mq = "/server.";

namespace core {
    namespace system {
        Mq::Mq()
        : m_mq_fd((mqd_t)-1) {
            m_mq_attrib = {.mq_maxmsg = MQ_MAXMSG, .mq_msgsize = MQ_CMD_MSGSIZE};
        }

        Mq::~Mq() {
            close();
        }

        bool Mq::open(const std::string name, int pid) {

            // message queue 이름 설정 ("mq_" + pid)
            m_mqname = make_mq_name(name, pid);
            if (name == CLIENT_MQ_NAME) {
                m_mq_attrib.mq_msgsize = MQ_CMD_MSGSIZE;
            }

            // message queue 생성
            m_mq_fd = ::mq_open(m_mqname.c_str(), O_CREAT|O_EXCL|O_RDWR, S_IRUSR|S_IWUSR, &m_mq_attrib);
            if (m_mq_fd == (mqd_t)-1) {
                // 이미 message queue가 생성되어 있으면 Open
                if (errno == EEXIST) {
                    m_mq_fd = ::mq_open(m_mqname.c_str(), O_RDWR);
                    if (m_mq_fd == (mqd_t)-1) {
                        syslog(LOG_ERR, "[Error : %s:%d] Failed : mq_open()! : %s",__FILE__, __LINE__, strerror(errno));
                        return false;
                    }
                } else {
                        syslog(LOG_ERR, "[Error : %s:%d] Failed : mq_open()! : %s",__FILE__, __LINE__, strerror(errno));
                    return false;
                }
            }
            return true;
        }
        
        void Mq::close(void) {
            // close message queue
            if (m_mq_fd != (mqd_t)-1) {
                mq_close(m_mq_fd);
            }
        }
        
        std::string Mq::make_mq_name(const std::string name, int pid) {
            std::string mq_name;
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
                syslog(LOG_ERR, "[Error : %s:%d] Failed : mq_send()! : %s",__FILE__, __LINE__, strerror(errno));
                return false;
            }
            return true;
        }

        int Mq::recv(DATA* r, size_t len) {
            int rcvByte = 0;

            if (mq_getattr(m_mq_fd, &m_mq_attrib) == -1) {
                syslog(LOG_ERR, "[Error : %s:%d] Failed : mq_getattr()! : %s",__FILE__, __LINE__, strerror(errno));
                return rcvByte;
            }

            if (m_mq_attrib.mq_curmsgs > 0) {
                rcvByte = mq_receive(m_mq_fd, (char*)r, len, 0);
                if (rcvByte < 0) {
                syslog(LOG_ERR, "[Error : %s:%d] Failed : mq_receive()! : %s",__FILE__, __LINE__, strerror(errno));
                    return 0;
                }
                syslog(LOG_DEBUG, "mq_receive : %d Bytes", rcvByte);
                common::print_hex(r, rcvByte);
            }
            return rcvByte;
        }
    }
}
