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
            strncpy(siteCode, code, 7);
        }
        //////////////////////////////////////////////
        // Class Command
        char* Command::getCommand() {
            char* command = new char[5] {0};
            strcpy(command, this->command);
            command[4] = '\0';
            return command;
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
        char Result::getRawResult() {
            return this->result;
        }
        std::string Result::getStrResult() {
            return this->result == 0x01? "Y" : "N";
        }
        void Result::setResult(char result) {
            this->result = result;            
        }
        /////////////////////////////////////////////
        // Class MsgHeader
        MsgHeader::MsgHeader() : stx(STX) {}
        MsgHeader::~MsgHeader() {}
        void MsgHeader::print() {
            syslog(LOG_DEBUG, "MsgHeader::print() ");
            syslog(LOG_DEBUG, "cmd: 0x%02X", this->cmd);
            syslog(LOG_DEBUG, "frm: 0x%04X", this->fromAddr.getAddr());
            syslog(LOG_DEBUG, "to : 0x%04X", this->toAddr.getAddr());
            syslog(LOG_DEBUG, "len: %u", this->length);
        }
        //////////////////////////////////////////////
        // Class MsgTail
        MsgTail::MsgTail() : etx(ETX) {}
        MsgTail::~MsgTail() {}
        void MsgTail::print() {
            syslog(LOG_DEBUG, "MsgTail::print() ");
            syslog(LOG_DEBUG, "crc: 0x%02X", this->crc8.getCRC8());
            syslog(LOG_DEBUG, "etx: 0x%02X", this->etx);
        }
        //////////////////////////////////////////////
        // bool Message::setFromAddr() {}
        // bool Message::setToAddr() {}
        // bool Message::setLength() {}
        // bool Message::calcCrc() {}
        //////////////////////////////////////////////
        // Class InitReq
        InitReq::InitReq() : stx(STX), cmd(INIT_REQ), length(7), etx(ETX) {}
        InitReq::~InitReq() {}
        void InitReq::print() {
            syslog(LOG_DEBUG, "InitReq::print() ");
            syslog(LOG_DEBUG, "cmd: 0x%02X", this->cmd);
            syslog(LOG_DEBUG, "frm: 0x%04X", this->fromAddr.getAddr());
            syslog(LOG_DEBUG, "to : 0x%04X", this->toAddr.getAddr());
            syslog(LOG_DEBUG, "len: %u", this->length);
            syslog(LOG_DEBUG, "scd: %s", this->siteCode.getSiteCode());
            syslog(LOG_DEBUG, "crc: 0x%02X", this->crc8.getCRC8());
            syslog(LOG_DEBUG, "etx: 0x%02X", this->etx);
        }
        //////////////////////////////////////////////
        // Class InitRes
        InitRes::InitRes() : stx(STX), cmd(INIT_RES), length(9), etx(ETX) {}
        InitRes::~InitRes() {}
        void InitRes::print() {
            syslog(LOG_DEBUG, "InitRes::print() ");
            syslog(LOG_DEBUG, "stx: 0x%02X", this->stx);
            syslog(LOG_DEBUG, "cmd: 0x%02X", this->cmd);
            syslog(LOG_DEBUG, "frm: 0x%04X", this->fromAddr.getAddr());
            syslog(LOG_DEBUG, "to : 0x%04X", this->toAddr.getAddr());
            syslog(LOG_DEBUG, "len: %u", this->length);
            syslog(LOG_DEBUG, "scd: %s", this->siteCode.getSiteCode());
            syslog(LOG_DEBUG, "rtu: 0x%04X", this->rtuAddr.getAddr());
            syslog(LOG_DEBUG, "crc: 0x%02X", this->crc8.getCRC8());
            syslog(LOG_DEBUG, "etx: 0x%02X", this->etx);
        }
        //////////////////////////////////////////////
        // Class HeartBeat
        HeartBeat::HeartBeat() : stx(STX), cmd(HEART_BEAT), length(0), etx(ETX) {}
        HeartBeat::~HeartBeat() {}
        void HeartBeat::print() {
            syslog(LOG_DEBUG, "HeartBeat::print() ");
            syslog(LOG_DEBUG, "stx: 0x%02X", this->stx);
            syslog(LOG_DEBUG, "cmd: 0x%02X", this->cmd);
            syslog(LOG_DEBUG, "frm: 0x%04X", this->fromAddr.getAddr());
            syslog(LOG_DEBUG, "to : 0x%04X", this->toAddr.getAddr());
            syslog(LOG_DEBUG, "len: %u", this->length);
            syslog(LOG_DEBUG, "crc: 0x%02X", this->crc8.getCRC8());
            syslog(LOG_DEBUG, "etx: 0x%02X", this->etx);
        }
        //////////////////////////////////////////////
        // Class HeartBeatAck
        HeartBeatAck::HeartBeatAck() : stx(STX), cmd(HEART_BEAT_ACK), length(0), etx(ETX) {}
        HeartBeatAck::~HeartBeatAck() {}
        void HeartBeatAck::print() {
            syslog(LOG_DEBUG, "HeartBeatAck::print() ");
            syslog(LOG_DEBUG, "stx: 0x%02X", this->stx);
            syslog(LOG_DEBUG, "cmd: 0x%02X", this->cmd);
            syslog(LOG_DEBUG, "frm: 0x%04X", this->fromAddr.getAddr());
            syslog(LOG_DEBUG, "to : 0x%04X", this->toAddr.getAddr());
            syslog(LOG_DEBUG, "len: %u", this->length);
            syslog(LOG_DEBUG, "crc: 0x%02X", this->crc8.getCRC8());
            syslog(LOG_DEBUG, "etx: 0x%02X", this->etx);
        }
        //////////////////////////////////////////////
        // Class CommandRtu
        CommandRtu::CommandRtu() : stx(STX), cmd(COMMAND_RTU), length(15), etx(ETX) {}
        CommandRtu::~CommandRtu() {}
        void CommandRtu::print() {
            syslog(LOG_DEBUG, "CommandRtu::print() ");
            syslog(LOG_DEBUG, "stx: 0x%02X", this->stx);
            syslog(LOG_DEBUG, "cmd: 0x%02X", this->cmd);
            syslog(LOG_DEBUG, "frm: 0x%04X", this->fromAddr.getAddr());
            syslog(LOG_DEBUG, "to : 0x%04X", this->toAddr.getAddr());
            syslog(LOG_DEBUG, "len: %u", this->length);
            syslog(LOG_DEBUG, "scd: %s", this->siteCode.getSiteCode());
            syslog(LOG_DEBUG, "dc : %s", this->dcCommand.getCommand());
            syslog(LOG_DEBUG, "ac : %s", this->acCommand.getCommand());
            syslog(LOG_DEBUG, "crc: 0x%02X", this->crc8.getCRC8());
            syslog(LOG_DEBUG, "etx: 0x%02X", this->etx);
        }
        //////////////////////////////////////////////
        // Class CommandRtuAck
        CommandRtuAck::CommandRtuAck() : stx(STX), cmd(COMMAND_RTU_ACK), length(16), etx(ETX) {}
        CommandRtuAck::~CommandRtuAck() {}
        void CommandRtuAck::print() {
            syslog(LOG_DEBUG, "CommandRtuAck::print() ");
            syslog(LOG_DEBUG, "stx: 0x%02X", this->stx);
            syslog(LOG_DEBUG, "cmd: 0x%02X", this->cmd);
            syslog(LOG_DEBUG, "frm: 0x%04X", this->fromAddr.getAddr());
            syslog(LOG_DEBUG, "to : 0x%04X", this->toAddr.getAddr());
            syslog(LOG_DEBUG, "len: %u", this->length);
            syslog(LOG_DEBUG, "scd: %s", this->siteCode.getSiteCode());
            syslog(LOG_DEBUG, "dc : %s", this->dcCommand.getCommand());
            syslog(LOG_DEBUG, "ac : %s", this->acCommand.getCommand());
            syslog(LOG_DEBUG, "rst: %0X", this->result.getRawResult());
            syslog(LOG_DEBUG, "crc: 0x%02X", this->crc8.getCRC8());
            syslog(LOG_DEBUG, "etx: 0x%02X", this->etx);
        }
        //////////////////////////////////////////////
        // Class ClientInitReq
        ClientInitReq::ClientInitReq() : stx(STX), cmd(CLIENT_INIT_REQ), length(0), etx(ETX) {}
        ClientInitReq::~ClientInitReq() {}
        void ClientInitReq::print() {
            syslog(LOG_DEBUG, "ClientInitReq::print() ");
            syslog(LOG_DEBUG, "stx: 0x%02X", this->stx);
            syslog(LOG_DEBUG, "cmd: 0x%02X", this->cmd);
            syslog(LOG_DEBUG, "frm: 0x%04X", this->fromAddr.getAddr());
            syslog(LOG_DEBUG, "to : 0x%04X", this->toAddr.getAddr());
            syslog(LOG_DEBUG, "len: %u", this->length);
            syslog(LOG_DEBUG, "crc: 0x%02X", this->crc8.getCRC8());
            syslog(LOG_DEBUG, "etx: 0x%02X", this->etx);
        }
        //////////////////////////////////////////////
        // Class ClientInitRes        
        ClientInitRes::ClientInitRes() : stx(STX), cmd(CLIENT_INIT_RES), length(2), etx(ETX) {}
        ClientInitRes::~ClientInitRes() {}
        void ClientInitRes::print() {
            syslog(LOG_DEBUG, "ClientInitRes::print() ");
            syslog(LOG_DEBUG, "stx: 0x%02X", this->stx);
            syslog(LOG_DEBUG, "cmd: 0x%02X", this->cmd);
            syslog(LOG_DEBUG, "frm: 0x%04X", this->fromAddr.getAddr());
            syslog(LOG_DEBUG, "to : 0x%04X", this->toAddr.getAddr());
            syslog(LOG_DEBUG, "len: %u", this->length);
            syslog(LOG_DEBUG, "clt: 0x%04X", this->clientAddr.getAddr());
            syslog(LOG_DEBUG, "crc: 0x%02X", this->crc8.getCRC8());
            syslog(LOG_DEBUG, "etx: 0x%02X", this->etx);
        }
        //////////////////////////////////////////////
        // Class CommandClient        
        CommandClient::CommandClient() : stx(STX), cmd(COMMAND_CLIENT), length(15), etx(ETX) {}
        CommandClient::~CommandClient() {}
        void CommandClient::print() {
            syslog(LOG_DEBUG, "CommandClient::print() ");
            syslog(LOG_DEBUG, "stx: 0x%02X", this->stx);
            syslog(LOG_DEBUG, "cmd: 0x%02X", this->cmd);
            syslog(LOG_DEBUG, "frm: 0x%04X", this->fromAddr.getAddr());
            syslog(LOG_DEBUG, "to : 0x%04X", this->toAddr.getAddr());
            syslog(LOG_DEBUG, "len: %u", this->length);
            syslog(LOG_DEBUG, "scd: %s", this->siteCode.getSiteCode());
            syslog(LOG_DEBUG, "dc : %s", this->dcCommand.getCommand());
            syslog(LOG_DEBUG, "ac : %s", this->acCommand.getCommand());
            syslog(LOG_DEBUG, "crc: 0x%02X", this->crc8.getCRC8());
            syslog(LOG_DEBUG, "etx: 0x%02X", this->etx);
        }
        //////////////////////////////////////////////
        // Class CommandClientAck
        CommandClientAck::CommandClientAck() : stx(STX), cmd(COMMAND_CLIENT_ACK), length(16), etx(ETX) {}
        CommandClientAck::~CommandClientAck() {}
        void CommandClientAck::print() {
            syslog(LOG_DEBUG, "CommandClientAck::print() ");
            syslog(LOG_DEBUG, "stx: 0x%02X", this->stx);
            syslog(LOG_DEBUG, "cmd: 0x%02X", this->cmd);
            syslog(LOG_DEBUG, "frm: 0x%04X", this->fromAddr.getAddr());
            syslog(LOG_DEBUG, "to : 0x%04X", this->toAddr.getAddr());
            syslog(LOG_DEBUG, "len: %u", this->length);
            syslog(LOG_DEBUG, "scd: %s", this->siteCode.getSiteCode());
            syslog(LOG_DEBUG, "dc : %s", this->dcCommand.getCommand());
            syslog(LOG_DEBUG, "ac : %s", this->acCommand.getCommand());
            syslog(LOG_DEBUG, "rst: %0X", this->result.getRawResult());
            syslog(LOG_DEBUG, "crc: 0x%02X", this->crc8.getCRC8());
            syslog(LOG_DEBUG, "etx: 0x%02X", this->etx);
        }
        //////////////////////////////////////////////
        // Class SetupInfo
        SetupInfo::SetupInfo() : stx(STX), cmd(SETUP_INFO), length(8), etx(ETX) {}
        SetupInfo::~SetupInfo() {}
        void SetupInfo::print() {
            syslog(LOG_DEBUG, "SetupInfo::print() ");
            syslog(LOG_DEBUG, "stx: 0x%02X", this->stx);
            syslog(LOG_DEBUG, "cmd: 0x%02X", this->cmd);
            syslog(LOG_DEBUG, "frm: 0x%04X", this->fromAddr.getAddr());
            syslog(LOG_DEBUG, "to : 0x%04X", this->toAddr.getAddr());
            syslog(LOG_DEBUG, "len: %u", this->length);
            syslog(LOG_DEBUG, "act: %0X", this->action.getAction());
            syslog(LOG_DEBUG, "scd: %s", this->siteCode.getSiteCode());
            syslog(LOG_DEBUG, "crc: 0x%02X", this->crc8.getCRC8());
            syslog(LOG_DEBUG, "etx: 0x%02X", this->etx);
        }
        //////////////////////////////////////////////
        // Class SetupInfoAck
        SetupInfoAck::SetupInfoAck() : stx(STX), cmd(SETUP_INFO_ACK), length(9), etx(ETX) {}
        SetupInfoAck::~SetupInfoAck() {}
        void SetupInfoAck::print() {
            syslog(LOG_DEBUG, "SetupInfoAck::print() ");
            syslog(LOG_DEBUG, "stx: 0x%02X", this->stx);
            syslog(LOG_DEBUG, "cmd: 0x%02X", this->cmd);
            syslog(LOG_DEBUG, "frm: 0x%04X", this->fromAddr.getAddr());
            syslog(LOG_DEBUG, "to : 0x%04X", this->toAddr.getAddr());
            syslog(LOG_DEBUG, "len: %u", this->length);
            syslog(LOG_DEBUG, "act: %0X", this->action.getAction());
            syslog(LOG_DEBUG, "scd: %s", this->siteCode.getSiteCode());
            syslog(LOG_DEBUG, "rst: %0X", this->result.getRawResult());
            syslog(LOG_DEBUG, "crc: 0x%02X", this->crc8.getCRC8());
            syslog(LOG_DEBUG, "etx: 0x%02X", this->etx);
        }
        //////////////////////////////////////////////
        // Class RtuStatusReq
        RtuStatusReq::RtuStatusReq() : stx(STX), cmd(RTU_STATUS_REQ), length(0), etx(ETX) {}
        RtuStatusReq::~RtuStatusReq() {}
        void RtuStatusReq::print() {
            syslog(LOG_DEBUG, "RtuStatusReq::print() ");
            syslog(LOG_DEBUG, "stx: 0x%02X", this->stx);
            syslog(LOG_DEBUG, "cmd: 0x%02X", this->cmd);
            syslog(LOG_DEBUG, "frm: 0x%04X", this->fromAddr.getAddr());
            syslog(LOG_DEBUG, "to : 0x%04X", this->toAddr.getAddr());
            syslog(LOG_DEBUG, "len: %u", this->length);
            syslog(LOG_DEBUG, "crc: 0x%02X", this->crc8.getCRC8());
            syslog(LOG_DEBUG, "etx: 0x%02X", this->etx);
        }
        //////////////////////////////////////////////
        // Class Status
        char Status::getStatus() {
            return this->status;
        }
        void Status::setStatus(DATA status) {
            this->status = status;
        }
        //////////////////////////////////////////////
        // Class RtuStatus
        // RtuStatus::RtuStatus() {}
        // RtuStatus::~RtuStatus() {}
        //////////////////////////////////////////////
        // Class RtuStatusResHead
        RtuStatusResHead::RtuStatusResHead() : stx(STX), cmd(RTU_STATUS_RES), length(0), count(0) {}
        RtuStatusResHead::~RtuStatusResHead() {}
        void RtuStatusResHead::print() {
            syslog(LOG_DEBUG, "RtuStatusResHead::print() ");
            syslog(LOG_DEBUG, "stx: 0x%02X", this->stx);
            syslog(LOG_DEBUG, "cmd: 0x%02X", this->cmd);
            syslog(LOG_DEBUG, "frm: 0x%04X", this->fromAddr.getAddr());
            syslog(LOG_DEBUG, "to : 0x%04X", this->toAddr.getAddr());
            syslog(LOG_DEBUG, "len: %u", this->length);
            syslog(LOG_DEBUG, "cnt: 0x%02X", this->count);
        }
        //////////////////////////////////////////////
        // Class RtuStatusResTail
        RtuStatusResTail::RtuStatusResTail() : etx(ETX) {}
        RtuStatusResTail::~RtuStatusResTail() {}
        void RtuStatusResTail::print() {
            syslog(LOG_DEBUG, "RtuStatusResTail::print() ");
            syslog(LOG_DEBUG, "crc: 0x%02X", this->crc8.getCRC8());
            syslog(LOG_DEBUG, "etx: 0x%02X", this->etx);
        }
        //////////////////////////////////////////////
    }
}