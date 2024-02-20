
#include "common.h"
#include "errorcode.h"

#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <dlfcn.h>
#include <pthread.h>

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