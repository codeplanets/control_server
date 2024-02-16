
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
                ST_LINGER stLinger;
                stLinger.l_onoff = 1;
                stLinger.l_linger = 5000;
                nRet = core::setsockopt(socket, SOL_SOCKET_, SO_LINGER_, (char*)&stLinger, sizeof(stLinger));
                if (SOCKET_ERROR_ == nRet) {
                    logWarn("core::setsockopt(socket, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger)) failure, %d", errno);
                }

                int bReuseAddr = 1;
                nRet = core::setsockopt(socket, SOL_SOCKET_, SO_REUSEADDR_, (char*)&bReuseAddr, sizeof(bReuseAddr));
                if (SOCKET_ERROR_ == nRet) {
                    logWarn("core::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (char*)&bReuseAddr, sizeof(bReuseAddr)) failure, %d", errno);
                }

                int bNodelay = 1;
                nRet = core::setsockopt(socket, IPPROTO_TCP_, TCP_NODELAY_, (char*)&bNodelay, sizeof(bNodelay));
                if (SOCKET_ERROR_ == nRet) {
                    logWarn("core::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (char*)&bNodelay, sizeof(bNodelay)) failure, %d", errno);
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
