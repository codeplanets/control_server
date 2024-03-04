#pragma once

#include "common.h"

namespace core {
    
    namespace formatter {

        class Address {
        private:
            u_short addr;   // big -> little endian
        public:
            bool setBigEndian(char* addr, int size) {
                if (size == 2) {
                    u_short x = 0;
                    memcpy(&x, addr, size);
                    std::cout << "BigEndian: " << x << std::endl;

                    x = x & 0xFF0F; // Section clear
                    x = ((x & 0xFF00) >> 8) | ((x & 0x00FF) << 8);

                    setLittleEndian(x);
                    std::cout << "LittleEndian: " << getAddr() << std::endl;

                    return true;
                }
                return false;
            }
        private:
            void setLittleEndian(u_short addr) { this->addr = addr; }
            u_short getAddr() { return addr;}
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
            char siteCode[8];

        public:
            char* getSiteCode() {
                return siteCode;
            }

            void setSiteCode(const char* code) {
                strcpy(siteCode, code);
                siteCode[7] = '\0';
                std::cout << "setSiteCode: " << getSiteCode() << std::endl;
            }
        };

        struct RSite {
            SiteCode siteCode;
            Address clientAddr;
            bool status;
            int pid;
        };
        
        class Command {
        private:
            char command[4];
        public:
            char getCommand(int);
            void setCommand(int, char);
        };

        class Action {
        private:
            char action;
        public:
            char getAction();
            void setAction(char);
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
            virtual ~Message(){;}
            virtual void print(){;}
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            SiteCode siteCode;
            DATA crc8;
            DATA etx;
        };

        class InitReq : public Message {
        public:
            ~InitReq(){}
            void print(){}
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            SiteCode siteCode;
            DATA crc8;
            DATA etx;
        };
        class InitRes : public Message {
        public:
            ~InitRes(){}
            void print(){}

            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            SiteCode siteCode;
            Address rtuAddr;
            DATA crc8;
            DATA etx;
        };
        class HeartBeat : public Message {
        public:
            ~HeartBeat(){;}
            void print(){;}
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            DATA crc8;
            DATA etx;
        };
        class HeartBeatAck : public Message {
        public:
            ~HeartBeatAck(){;}
            void print(){;}
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            DATA crc8;
            DATA etx;
        };
        class CommandRtu : public Message {
        public:
            ~CommandRtu(){;}
            void print(){;}
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            SiteCode siteCode;
            Command dcCommand;
            Command acCommand;
            DATA crc8;
            DATA etx;
        };
        class CommandRtuAck : public Message {
        public:
            ~CommandRtuAck(){;}
            void print(){;}
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            SiteCode siteCode;
            Command dcCommand;
            Command acCommand;
            CommandResult result;
            DATA crc8;
            DATA etx;
        };
        class ClientInitReq : public Message {
        public:
            ~ClientInitReq(){;}
            void print(){;}
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            DATA crc8;
            DATA etx;
        };
        class ClientInitRes : public Message {
        public:
            ~ClientInitRes(){;}
            void print(){;}
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            Address clientAddr;
            DATA crc8;
            DATA etx;
        };
        class CommandClient : public Message {
        public:
            ~CommandClient(){;}
            void print(){;}
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            SiteCode siteCode;
            Command dcCommand;
            Command acCommand;
            DATA crc8;
            DATA etx;
        };
        class CommandClientAck : public Message {
        public:
            ~CommandClientAck(){;}
            void print(){;}
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            SiteCode siteCode;
            Command dcCommand;
            Command acCommand;
            CommandResult result;
            DATA crc8;
            DATA etx;
        };
        class SetupInfo : public Message {
        public:
            ~SetupInfo(){;}
            void print(){;}
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            Action action;
            SiteCode siteCode;
            DATA crc8;
            DATA etx;
        };
        class SetupInfoAck : public Message {
        public:
            ~SetupInfoAck(){;}
            void print(){;}
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            Action action;
            SiteCode siteCode;
            ActionResult result;
            DATA crc8;
            DATA etx;
        };
        class RtuStatusReq : public Message {
        public:
            ~RtuStatusReq(){;}
            void print(){;}
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            DATA crc8;
            DATA etx;
        };

        class Status {
        public:
            DATA status;
        };
        class RtuStatus {
        public:
            SiteCode siteCode;
            Status status;
        };

        class RtuStatusRes : public Message {
        public:
            ~RtuStatusRes(){;}
            void print(){;}
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            unsigned short count;
            RtuStatus rtuStatus;
            DATA crc8;
            DATA etx;
        };
    }

}
