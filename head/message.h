#pragma once
#pragma pack(1)

#include "common.h"

namespace core {
    
    namespace formatter {

        class Address {
        private:
            DATA addr[2];   // big -> little endian
        public:
            bool setAddr(DATA* addr, int size);
            void setAddr(u_short addr, char section = DEFAULT_ADDRESS);
            void setAddr(std::string addr, char section = DEFAULT_ADDRESS);
            u_short getAddr();
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
            char* getSiteCode();
            void setSiteCode(const char* code);
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
            char* getCommand();
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

        class InitReq {
        public:
            InitReq();
            ~InitReq();
            void print();

            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            SiteCode siteCode;
            CRC crc8;
            DATA etx;
        };

        struct RInitReq {
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            SiteCode siteCode;
            Address rtuAddr;
            CRC crc8;
            DATA etx;
        };

        class InitRes {
        public:
            InitRes();
            ~InitRes();
            void print();

            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length; // Payload + crc + etx
            SiteCode siteCode;
            Address rtuAddr;
            CRC crc8; // stx + cmd + fromAddr + toAddr + length + payload
            DATA etx;
        };
        class HeartBeat {
        public:
            HeartBeat();
            ~HeartBeat();
            void print();
            
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            CRC crc8;
            DATA etx;
        };
        class HeartBeatAck {
        public:
            HeartBeatAck();
            ~HeartBeatAck();
            void print();
            
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            CRC crc8;
            DATA etx;
        };
        class CommandRtu {
        public:
            CommandRtu();
            ~CommandRtu();
            void print();
            
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            SiteCode siteCode;
            Command dcCommand;
            Command acCommand;
            CRC crc8;
            DATA etx;
        };
        class CommandRtuAck {
        public:
            CommandRtuAck();
            ~CommandRtuAck();
            void print();
            
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            SiteCode siteCode;
            Command dcCommand;
            Command acCommand;
            CommandResult result;
            CRC crc8;
            DATA etx;
        };
        class ClientInitReq {
        public:
            ClientInitReq();
            ~ClientInitReq();
            void print();
            
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            CRC crc8;
            DATA etx;
        };
        class ClientInitRes {
        public:
            ClientInitRes();
            ~ClientInitRes();
            void print();
            
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            Address clientAddr;
            CRC crc8;
            DATA etx;
        };
        class CommandClient {
        public:
            CommandClient();
            ~CommandClient();
            void print();
            
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            SiteCode siteCode;
            Command dcCommand;
            Command acCommand;
            CRC crc8;
            DATA etx;
        };
        class CommandClientAck {
        public:
            CommandClientAck();
            ~CommandClientAck();
            void print();
            
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            SiteCode siteCode;
            Command dcCommand;
            Command acCommand;
            CommandResult result;
            CRC crc8;
            DATA etx;
        };
        class SetupInfo {
        public:
            SetupInfo();
            ~SetupInfo();
            void print();
            
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            Action action;
            SiteCode siteCode;
            CRC crc8;
            DATA etx;
        };
        class SetupInfoAck {
        public:
            SetupInfoAck();
            ~SetupInfoAck();
            void print();
            
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            Action action;
            SiteCode siteCode;
            ActionResult result;
            CRC crc8;
            DATA etx;
        };
        class RtuStatusReq {
        public:
            RtuStatusReq();
            ~RtuStatusReq();
            void print();
            
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            CRC crc8;
            DATA etx;
        };

        class Status {
        public:
            DATA status;
        };
        class RtuStatus {
        public:
            RtuStatus(SiteCode scd, Status stat);
            SiteCode siteCode;
            Status status;
        };

        class RtuStatusResHead {
        public:
            RtuStatusResHead();
            ~RtuStatusResHead();
            void print();
            
            DATA stx;
            DATA cmd;
            Address fromAddr;
            Address toAddr;
            unsigned short length;
            unsigned short count;
            // std::vector<RtuStatus> rtuStatus;
            RtuStatus rtuStatus[];
        };
        class RtuStatusResTail {
        public:
            RtuStatusResTail();
            ~RtuStatusResTail();
            void print();

            CRC crc8;
            DATA etx;
        };
    }
}
