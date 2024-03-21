
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
                unsigned int microSec = (milliSec - sec * 1000) * 1000;
                ::sleep(sec);
                ::usleep(microSec);
            }
        }
        
        void print_hex(DATA *buf, int size) {
            for (int index = 0; index < size; index++) {
                if (index % 16 == 0) printf("\n");
                printf("0x%02X ", buf[index]);
            }
            printf("\n");
        }

        u_short convert_be_to_le(DATA* be, int size) {
            if (size == 2) {
                u_short x = 0;
                memcpy(&x, be, size);
                // std::cout << "BigEndian: " << x << std::endl;
                // x = x & 0xFF0F; // Section clear
                x = ((x & 0xFF00) >> 8) | ((x & 0x00FF) << 8);
                // std::cout << "LittleEndian: " << x << std::endl;
                return x;
            }
            return 0;
        }
        DATA calcCRC(DATA *buf, int size) {
            if (size < 10) {
                return 0x00;
            }

            DATA CRC = 0x00;
            int len = size - 2;

            for (int i = 0; i < len; i++) {
                CRC ^= *(buf + i);
            }
            CRC = CRC & 0x7F;

            return CRC;
        }

        bool checkCRC(DATA *buf, int size, DATA crc) {
            if (size < 10) {
                return false;
            }

            if (crc != calcCRC(buf, size)) {
                return false;
            }

            return true;
        }

    }
	
}