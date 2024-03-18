#pragma once

#include <mqueue.h>     // mq_attr, mqd_t, mq_open() , mq_close(), mq_send(), mq_receive(), mq_unlink()

#include "common.h"
 
using namespace std;

namespace core {
    namespace system {
        class Mq {
        protected:
            string m_mqname;
            struct mq_attr m_mq_attrib;
            mqd_t m_mq_fd;

        private:
            string make_mq_name(const string name, int pid);
            string get_mq_name();

        public:
            Mq();
            virtual ~Mq();

            bool open(const string name, int pid);
            void close(void);
            
            bool send(DATA*, size_t len);
            int recv(DATA*, size_t len);
        };
    }
}