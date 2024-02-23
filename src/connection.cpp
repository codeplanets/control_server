
#include "connection.h"

namespace core {
    namespace server {
        Connection::Connection(pid_t pid, ServerSocket* pSocket)
            : m_pSocket(pSocket)
            , m_pid(pid) {
        }
        Connection::~Connection() {
            
        }
    }
}
