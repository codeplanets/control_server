#include "rtu.h"

namespace core {
    RTUclient::RTUclient() {}
    RTUclient::~RTUclient() {}

    void RTUclient::init(ServerSocket& sock) { newSock = sock; }

    /**
     * @return true if SiteCode is available, false otherwise
    */
    bool RTUclient::isSiteCodeAvailable() { return true; }
    void RTUclient::createMessageQueue() {}
    bool RTUclient::updateStatus(bool status) { return true; }

    const DATA* RTUclient::reqMessage(DATA cmd) { return NULL; }  // INIT_RES, 

    void RTUclient::run() {}
}
