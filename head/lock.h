#pragma once

#include <semaphore.h>

#include "common.h"

#define SEM_NAME "SemLock"

namespace core {
    namespace system {
        class SemLock {
        private:
            sem_t* sem;
            u_int value;
            std::string sem_name;
            time_t timeout;
            struct timespec tm;
        public:
            SemLock(const std::string& sem_name, time_t timeout = 10);
            ~SemLock();
        
            bool lock();
            bool unlock();
        };
    }
}