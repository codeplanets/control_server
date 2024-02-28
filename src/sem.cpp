#include <iostream>
#include <fcntl.h>   // O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#include "sem.h"

using namespace std;

sem_t* gRunning = SEM_FAILED;

namespace core {
    namespace common {

        bool isRunning() {
            bool bRet = false;
            gRunning = ::sem_open(SERVER_SEMAPHORE, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, 1);
            if (gRunning == SEM_FAILED) {
                if (errno == EEXIST) {
                    bRet = true;
                    // sem_close(gRunning);
                    // sem_unlink(SERVER_SEMAPHORE);
                }
            }
            return bRet;
        }

        void close_sem() {
            if (gRunning != SEM_FAILED) {
                sem_close(gRunning);
                sem_unlink(SERVER_SEMAPHORE);
            }
        }
    }
}
