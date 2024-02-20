#pragma once

#include "packetizer.h"
#include <iostream>

namespace core {
    namespace server {
        class Protocol {
        protected:
            TcpSocket* m_pSocket;

        public:
            Protocol(TcpSocket* pSocket);
            virtual ~Protocol(void);

            virtual ECODE connect(std::string strIP, unsigned short port, unsigned int timeOut);
            virtual void close(void);

            template<typename T>
            ECODE sendPacket(T* pPacket, unsigned int timeOut = 30000);
            template<typename T>
            ECODE recvPacket(T* pPacket, unsigned int timeOut = 30000);
        };
    }
}