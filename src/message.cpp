#include "message.h"
#include <iostream>

namespace core {
    
    namespace formatter {
        //////////////////////////////////////////////
        // Class Address
        bool Address::setAddr(DATA* addr, int size) {
            if (size == 2) {
                memcpy(this->addr, addr, size);
                return true;
            }
            return false;
        }
        void Address::setAddr(u_short addr, char section) {   // 수신 메시지에서 읽어온 주소 변환
            DATA ch[2];
            ch[0]=(char)(addr >> 8) | section; // | 0x10 주의
            ch[1]=(char)(addr & 0x00ff);
            setAddr(ch, 2);
        }
        void Address::setAddr(std::string addr, char section) {   // DB에서 읽어온 주소 변환
            u_short num = std::stoi(addr);
            setAddr(num, section);
        }
        u_short Address::getAddr() {
            return common::convert_be_to_le(addr, 2);
        }
        //////////////////////////////////////////////
        // Class CRC
        DATA CRC::getCRC8() {
            return this->crc8;
        }
        void CRC::setCRC8(DATA crc) {
            this->crc8 = crc;
        }
        //////////////////////////////////////////////
        // Class SiteCode
        char* SiteCode::getSiteCode() {
            char* siteCode = new char[8] {0};
            strcpy(siteCode, this->siteCode);
            siteCode[7] = '\0';
            return siteCode;
        }
        void SiteCode::setSiteCode(const char* code) {
            strcpy(siteCode, code);
            std::cout << "setSiteCode: " << getSiteCode() << std::endl;
        }
        //////////////////////////////////////////////
        // Class Command
        char* Command::getCommand() {
            return this->command;
        }
        char Command::getCommand(int idx) {
            return this->command[idx];
        }
        void Command::setCommand(int idx, char state) {
            this->command[idx] = state;
        }
        //////////////////////////////////////////////
        // Class Action
        char Action::getAction() {
            return this->action;
        }
        void Action::setAction(char action) {
            this->action = action;
        }
        /////////////////////////////////////////////
        // Class Result        
        char Result::getResult() {
            return this->result;
        }
        void Result::setResult(char result) {
            this->result = result;            
        }
        //////////////////////////////////////////////
        // bool Message::setFromAddr() {}
        // bool Message::setToAddr() {}
        // bool Message::setLength() {}
        // bool Message::calcCrc() {}
        //////////////////////////////////////////////
        // Class InitReq
        InitReq::InitReq() : stx(STX), cmd(INIT_REQ), length(9), etx(ETX) {}
        InitReq::~InitReq() {}
        void InitReq::print() {
            std::cout << "InitReq: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            printf("frm: 0x%04x \n", this->fromAddr.getAddr());
            printf("to : 0x%04x \n", this->toAddr.getAddr());
            printf("len: %u \n", this->length);
            std::cout << "scd: " << this->siteCode.getSiteCode() << std::endl;
            printf("crc: 0x%02x \n", this->crc8.getCRC8());
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class InitRes
        InitRes::InitRes() : stx(STX), cmd(INIT_RES), length(11), etx(ETX) {}
        InitRes::~InitRes() {}
        void InitRes::print() {
            std::cout << "InitReq: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            printf("frm: 0x%04x \n", this->fromAddr.getAddr());
            printf("to : 0x%04x \n", this->toAddr.getAddr());
            printf("len: %u \n", this->length);
            std::cout << "scd: " << this->siteCode.getSiteCode() << std::endl;
            printf("rtu: 0x%04x \n", this->rtuAddr.getAddr());
            printf("crc: 0x%02x \n", this->crc8.getCRC8());
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class HeartBeat
        HeartBeat::HeartBeat() : stx(STX), cmd(HEART_BEAT), length(2), etx(ETX) {}
        HeartBeat::~HeartBeat() {}
        void HeartBeat::print() {
            std::cout << "HeartBeat: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            printf("frm: 0x%04x \n", this->fromAddr.getAddr());
            printf("to : 0x%04x \n", this->toAddr.getAddr());
            printf("len: %u \n", this->length);
            printf("crc: 0x%02x \n", this->crc8.getCRC8());
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class HeartBeatAck
        HeartBeatAck::HeartBeatAck() : stx(STX), cmd(HEART_BEAT_ACK), length(2), etx(ETX) {}
        HeartBeatAck::~HeartBeatAck() {}
        void HeartBeatAck::print() {
            std::cout << "HeartBeatAck: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            printf("frm: 0x%04x \n", this->fromAddr.getAddr());
            printf("to : 0x%04x \n", this->toAddr.getAddr());
            printf("len: %u \n", this->length);
            printf("crc: 0x%02x \n", this->crc8.getCRC8());
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class CommandRtu
        CommandRtu::CommandRtu() : stx(STX), cmd(COMMAND_RTU), length(17), etx(ETX) {}
        CommandRtu::~CommandRtu() {}
        void CommandRtu::print() {
            std::cout << "CommandRtu: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            printf("frm: 0x%04x \n", this->fromAddr.getAddr());
            printf("to : 0x%04x \n", this->toAddr.getAddr());
            printf("len: %u \n", this->length);
            std::cout << "scd: " << this->siteCode.getSiteCode() << std::endl;
            std::cout << "dc : " << this->dcCommand.getCommand() << std::endl;
            std::cout << "ac : " << this->acCommand.getCommand() << std::endl;
            printf("crc: 0x%02x \n", this->crc8.getCRC8());
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class CommandRtuAck
        CommandRtuAck::CommandRtuAck() : stx(STX), cmd(COMMAND_RTU_ACK), length(18), etx(ETX) {}
        CommandRtuAck::~CommandRtuAck() {}
        void CommandRtuAck::print() {
            std::cout << "CommandRtu: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            printf("frm: 0x%04x \n", this->fromAddr.getAddr());
            printf("to : 0x%04x \n", this->toAddr.getAddr());
            printf("len: %u \n", this->length);
            std::cout << "scd: " << this->siteCode.getSiteCode() << std::endl;
            std::cout << "dc : " << this->dcCommand.getCommand() << std::endl;
            std::cout << "ac : " << this->acCommand.getCommand() << std::endl;
            std::cout << "rst: " << this->result.getResult() << std::endl;
            printf("crc: 0x%02x \n", this->crc8.getCRC8());
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class ClientInitReq
        ClientInitReq::ClientInitReq() : stx(STX), cmd(CLIENT_INIT_REQ), length(2), etx(ETX) {}
        ClientInitReq::~ClientInitReq() {}
        void ClientInitReq::print() {
            std::cout << "CommandRtu: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            printf("frm: 0x%04x \n", this->fromAddr.getAddr());
            printf("to : 0x%04x \n", this->toAddr.getAddr());
            printf("len: %u \n", this->length);
            printf("crc: 0x%02x \n", this->crc8.getCRC8());
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class ClientInitRes        
        ClientInitRes::ClientInitRes() : stx(STX), cmd(CLIENT_INIT_RES), length(4), etx(ETX) {}
        ClientInitRes::~ClientInitRes() {}
        void ClientInitRes::print() {
            std::cout << "CommandRtu: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            printf("frm: 0x%04x \n", this->fromAddr.getAddr());
            printf("to : 0x%04x \n", this->toAddr.getAddr());
            printf("len: %u \n", this->length);
            printf("clt: 0x%04x \n", this->clientAddr.getAddr());
            printf("crc: 0x%02x \n", this->crc8.getCRC8());
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class CommandClient        
        CommandClient::CommandClient() : stx(STX), cmd(COMMAND_CLIENT), length(17), etx(ETX) {}
        CommandClient::~CommandClient() {}
        void CommandClient::print() {
            std::cout << "CommandRtu: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            printf("frm: 0x%04x \n", this->fromAddr.getAddr());
            printf("to : 0x%04x \n", this->toAddr.getAddr());
            printf("len: %u \n", this->length);
            std::cout << "scd: " << this->siteCode.getSiteCode() << std::endl;
            std::cout << "dc : " << this->dcCommand.getCommand() << std::endl;
            std::cout << "ac : " << this->acCommand.getCommand() << std::endl;
            printf("crc: 0x%02x \n", this->crc8.getCRC8());
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class CommandClientAck
        CommandClientAck::CommandClientAck() : stx(STX), cmd(COMMAND_CLIENT_ACK), length(18), etx(ETX) {}
        CommandClientAck::~CommandClientAck() {}
        void CommandClientAck::print() {
            std::cout << "CommandRtu: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            printf("frm: 0x%04x \n", this->fromAddr.getAddr());
            printf("to : 0x%04x \n", this->toAddr.getAddr());
            printf("len: %u \n", this->length);
            std::cout << "scd: " << this->siteCode.getSiteCode() << std::endl;
            std::cout << "dc : " << this->dcCommand.getCommand() << std::endl;
            std::cout << "ac : " << this->acCommand.getCommand() << std::endl;
            std::cout << "rst: " << this->result.getResult() << std::endl;
            printf("crc: 0x%02x \n", this->crc8.getCRC8());
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class SetupInfo
        SetupInfo::SetupInfo() : stx(STX), cmd(SETUP_INFO), length(10), etx(ETX) {}
        SetupInfo::~SetupInfo() {}
        void SetupInfo::print() {
            std::cout << "CommandRtu: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            printf("frm: 0x%04x \n", this->fromAddr.getAddr());
            printf("to : 0x%04x \n", this->toAddr.getAddr());
            printf("len: %u \n", this->length);
            std::cout << "act: " << this->action.getAction() << std::endl;
            std::cout << "scd: " << this->siteCode.getSiteCode() << std::endl;
            printf("crc: 0x%02x \n", this->crc8.getCRC8());
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class SetupInfoAck
        SetupInfoAck::SetupInfoAck() : stx(STX), cmd(SETUP_INFO_ACK), length(11), etx(ETX) {}
        SetupInfoAck::~SetupInfoAck() {}
        void SetupInfoAck::print() {
            std::cout << "CommandRtu: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            printf("frm: 0x%04x \n", this->fromAddr.getAddr());
            printf("to : 0x%04x \n", this->toAddr.getAddr());
            printf("len: %u \n", this->length);
            std::cout << "act: " << this->action.getAction() << std::endl;
            std::cout << "scd: " << this->siteCode.getSiteCode() << std::endl;
            std::cout << "rst: " << this->result.getResult() << std::endl;
            printf("crc: 0x%02x \n", this->crc8.getCRC8());
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class RtuStatusReq
        RtuStatusReq::RtuStatusReq() : stx(STX), cmd(RTU_STATUS_REQ), length(2), etx(ETX) {}
        RtuStatusReq::~RtuStatusReq() {}
        void RtuStatusReq::print() {
            std::cout << "CommandRtu: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            printf("frm: 0x%04x \n", this->fromAddr.getAddr());
            printf("to : 0x%04x \n", this->toAddr.getAddr());
            printf("len: %u \n", this->length);
            printf("crc: 0x%02x \n", this->crc8.getCRC8());
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class RtuStatusRes
        RtuStatusRes::RtuStatusRes() : stx(STX), cmd(RTU_STATUS_RES), etx(ETX) {}
        RtuStatusRes::~RtuStatusRes() {}
        void RtuStatusRes::print() {
            std::cout << "CommandRtu: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            printf("frm: 0x%04x \n", this->fromAddr.getAddr());
            printf("to : 0x%04x \n", this->toAddr.getAddr());
            printf("len: %u \n", this->length);
            std::cout << "cnt: " << this->count << std::endl;
            printf("crc: 0x%02x \n", this->crc8.getCRC8());
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
    }
}