
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
        
        void print_hex(char *buf, int size) {
            int index;

            for (index = 0; index < size; index++) {
                if (index % 16 == 0) std::cout << std::endl;
                printf("0x%02X ", buf[index]);
            }
            std::cout << std::endl;
        }
    }
	
}