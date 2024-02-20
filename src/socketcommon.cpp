
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "socketcommon.h"
#include "socket.h"
#include "socketdefine.h"
#include "log.h"
using namespace core::system;

namespace core {
    namespace server {
        ECODE setCommonSockOpt(SOCKET socket) {
            ECODE nRet = EC_SUCCESS;
            try {
                struct linger stLinger;
                stLinger.l_onoff = 1;
                stLinger.l_linger = 5000;
                nRet = setsockopt(socket, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));
                if (SOCKET_ERROR_ == nRet) {
                    logWarn("core::setsockopt(socket, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger)) failure, %d", errno);
                }

                int bReuseAddr = 1;
                nRet = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (char*)&bReuseAddr, sizeof(bReuseAddr));
                if (SOCKET_ERROR_ == nRet) {
                    logWarn("core::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (char*)&bReuseAddr, sizeof(bReuseAddr)) failure, %d", errno);
                }
            } catch (std::exception& e) {
                nRet = errno;
                logError("%s", e.what());
                return nRet;
            }

            return EC_SUCCESS;
        }
    }
}
