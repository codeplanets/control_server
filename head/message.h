#pragma once

#include "common.h"

namespace core {
    
    namespace formatter {

        class Address {
        private:
            DATA addr[2];
        public:
            unsigned short getBigEndian();
            void setBigEndian(unsigned short);
            unsigned short getLittleEndian();
            void setLittleEndian(unsigned short);
        };

        class CRC {
        private:
            DATA crc8;
        public:
            DATA getCRC8();
            void setCRC8(DATA);
        };
        
        class SiteCode {
        private:
            char siteCode[7];
        public:
            char getSiteCode();
            void setSiteCode(char);
        };

        class Command {
        private:
            char command[4];
        public:
            char getCommand(int);
            void setCommand(int, char);
        };

        class Result {
        protected:
            char result;
        public:
            virtual char getResult();
            virtual void setResult(char);
        };

        class CommandResult : public Result {
        
        };

        class ActionResult : public Result {

        };

        class Message {
        public:
            virtual ~Message() = 0;

            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            DATA payload[MAX_RAW_BUFF - 2];
            DATA crc8;
            DATA etx;
        };

        class InitReq : public Message {
        public:
            virtual ~InitReq() = 0;
        };
        class InitRes : public Message {
        public:
            virtual ~InitRes() = 0;
        };
        class HeartBeat : public Message {
        public:
            virtual ~HeartBeat() = 0;
        };
        class HeartBeatAck : public Message {
        public:
            virtual ~HeartBeatAck() = 0;
        };
        class CommandRtu : public Message {
        public:
            virtual ~CommandRtu() = 0;
        };
        class CommandRtuAck : public Message {
        public:
            virtual ~CommandRtuAck() = 0;
        };
        class ClientInitReq : public Message {
        public:
            virtual ~ClientInitReq() = 0;
        };
        class ClientInitRes : public Message {
        public:
            virtual ~ClientInitRes() = 0;
        };
        class CommandClient : public Message {
        public:
            virtual ~CommandClient() = 0;
        };
        class CommandClientAck : public Message {
        public:
            virtual ~CommandClientAck() = 0;
        };
        class SetupInfo : public Message {
        public:
            virtual ~SetupInfo() = 0;
        };
        class SetupInfoAck : public Message {
        public:
            virtual ~SetupInfoAck() = 0;
        };
        class RtuStatusReq : public Message {
        public:
            virtual ~RtuStatusReq() = 0;
        };
        class RtuStatusRes : public Message {
        public:
            virtual ~RtuStatusRes() = 0;
        };
    }

}
