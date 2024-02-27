#pragma once

#include <mqueue.h>     // mq_attr, mqd_t, mq_open() , mq_close(), mq_send(), mq_receive(), mq_unlink()

#include "common.h"
 
using namespace std;

const char* name_posix_mq = "/mq_";

namespace core {
    namespace system {
        class Mq {
        protected:
            struct mq_attr mq_attrib;
            mqd_t mq_fd;

        private:
            string get_mq_name(const char* name, int pid);

        public:
            Mq();
            virtual ~Mq();

            bool open();
            void close(void);
            
            const Mq& operator << (const DATA*) const;
            const Mq& operator >> (DATA*) const;
        };
    }
}