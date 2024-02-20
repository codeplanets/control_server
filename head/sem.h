#pragma once

#include <semaphore.h>
#define SERVER_SEMAPHORE "control_server"

namespace core {
    namespace common {
        bool isRunning();
        void close_sem();
    }
}