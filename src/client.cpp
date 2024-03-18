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
        if (m_isCreatedMq) {
            mq.close();
        }
        updateStatus(false);
    }

    bool Client::createMessageQueue(std::string mq_name) {
        bool isCreated = mq.open(mq_name, getpid());
        m_isCreatedMq = isCreated;
        return isCreated;
    }
    
    bool Client::updateStatus(bool status) { return true; }
    bool Client::insertDatabase(bool status) { return true; }
    bool Client::updateDatabase(bool status) { return true; }
}
