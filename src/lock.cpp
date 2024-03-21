
#include <fcntl.h>

#include "lock.h"

namespace core {
    namespace system {
        SemLock::SemLock(const std::string& name, time_t timeout) : value(1), sem_name(name), timeout(10) {
            sem = sem_open(sem_name.c_str(), O_CREAT | O_EXCL | O_RDWR, 0666, 1);
            if (sem == SEM_FAILED) {
                if (errno != EEXIST) {
                    throw std::runtime_error("sem_open failed");
                } else {
                    sem = sem_open(sem_name.c_str(), 0);
                    if (sem == SEM_FAILED) {
                        throw std::runtime_error("sem_open failed");
                    }
                }
            }
        }
        SemLock::~SemLock() {
            sem_destroy(sem);
        }
    
        bool SemLock::lock() {
            if (sem) {
                clock_gettime(CLOCK_REALTIME, &tm);
                tm.tv_sec += timeout;

                if (!sem_timedwait(sem, &tm)) {
                    return true;
                }
            }

            return false;
        }
        bool SemLock::unlock() {
            if (sem) {
                if (!sem_post(sem)) {
                    return true;
                }
            }
            return false;
        }
    }
}