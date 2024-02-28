
#include "common.h"
#include "errorcode.h"

#include <unistd.h>

namespace core {
    namespace common {
        void sleep(unsigned int milliSec) {
            if( milliSec < 1000000 ) {
                ::usleep(milliSec * 1000);
            } else {
                unsigned int sec = milliSec / 1000;
                unsigned int microSec = (microSec - sec * 1000) * 1000;
                ::sleep(sec);
                ::usleep(microSec);
            }
        }
    }
	
}