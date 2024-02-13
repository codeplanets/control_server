#pragma once

#include "socket.h"
#include "packetizer.h"

namespace core {
    namespace server {
        class Protocol {
        protected:
            core::server:CTcpSocket* m_pSocket;

        public:
            Protocol(core::server:CTcpSocket* pSocket);
            virtual ~Protocol(void);

            virtual ECODE Connect(std::string strIP, WORD wPort, DWORD dwTimeOut);
            virtual void Close(void);

            template<typename T>
            ECODE SendPacket(T* pPacket, DWORD dwTimeOut = 30000) = 0;
            template<typename T>
            ECODE RecvPacket(T* pPacket, DWORD dwTimeOut = 30000) = 0;

        protected:
            virtual ECODE SendPacket(DWORD dwID, core::IFormatterObjectA* pPacket, DWORD dwTimeOut) = 0;
            virtual ECODE RecvPacket(DWORD dwID, core::IFormatterObjectA* pPacket, DWORD dwTimeOut) = 0;
        };
    }
}