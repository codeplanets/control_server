
#include "connection.h"

namespace core {
    namespace server {
        Connection::Connection(TcpSocket* pSocket)
            : m_pSocket(pSocket)
		    , m_strClientIP() {
        }
        Connection::~Connection() {
            
        }
    }
}