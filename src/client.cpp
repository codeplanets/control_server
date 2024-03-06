#include <unistd.h>

#include "client.h"
#include "packetizer.h"
#include "socketexception.h"

namespace core {
    Client::Client(ServerSocket& sock)
    : newSock(sock)
    , m_isCreatedMq(false) {
        
    }
    
    Client::~Client() {
        updateStatus(false);
    }

    void Client::init(ServerSocket& sock) {
        newSock = sock;
    }

    bool Client::createMessageQueue() {
        bool isCreated = mq.open(getpid());
        m_isCreatedMq = isCreated;
        return isCreated;
    }
    
    bool Client::updateStatus(bool status) { return true; }
    bool Client::insertDatabase(bool status) { return true; }
    bool Client::updateDatabase(bool status) { return true; }
}
