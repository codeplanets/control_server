#include "message.h"

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
        u_short Address::getAddr() {
            return common::convert_be_to_le(addr, 2);
        }
        //////////////////////////////////////////////
        // Class SiteCode
        char* SiteCode::getSiteCode() {
            return this->siteCode;
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
        InitReq::InitReq() : stx(STX), etx(ETX), cmd(INIT_REQ), length(9) {}
        InitReq::~InitReq() {}
        void InitReq::print() {
            std::cout << "InitReq: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            std::cout << "frm: " << this->fromAddr.getAddr() << std::endl;
            std::cout << "to: " << this->toAddr.getAddr() << std::endl;
            std::cout << "len: " << this->length << std::endl;
            std::cout << "scd: " << this->siteCode.getSiteCode() << std::endl;
            printf("crc8: 0x%02x \n", this->crc8);
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class InitRes
        InitRes::InitRes() : stx(STX), etx(ETX), cmd(INIT_RES), length(11) {}
        InitRes::~InitRes() {}
        void InitRes::print() {
            std::cout << "InitReq: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            std::cout << this->fromAddr.getAddr() << std::endl;
            std::cout << this->toAddr.getAddr() << std::endl;
            std::cout << this->length << std::endl;
            std::cout << this->siteCode.getSiteCode() << std::endl;
            std::cout << this->rtuAddr.getAddr() << std::endl;
            printf("crc8: 0x%02x \n", this->crc8);
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class HeartBeat
        HeartBeat::HeartBeat() : stx(STX), etx(ETX), cmd(HEART_BEAT), length(2) {}
        HeartBeat::~HeartBeat() {}
        void HeartBeat::print() {
            std::cout << "HeartBeat: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            std::cout << this->fromAddr.getAddr() << std::endl;
            std::cout << this->toAddr.getAddr() << std::endl;
            std::cout << this->length << std::endl;
            printf("crc8: 0x%02x \n", this->crc8);
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class HeartBeatAck
        HeartBeatAck::HeartBeatAck() : stx(STX), etx(ETX), cmd(HEART_BEAT_ACK), length(2) {}
        HeartBeatAck::~HeartBeatAck() {}
        void HeartBeatAck::print() {
            std::cout << "HeartBeatAck: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            std::cout << this->fromAddr.getAddr() << std::endl;
            std::cout << this->toAddr.getAddr() << std::endl;
            std::cout << this->length << std::endl;
            printf("crc8: 0x%02x \n", this->crc8);
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class CommandRtu
        CommandRtu::CommandRtu() : stx(STX), etx(ETX), cmd(COMMAND_RTU), length(17) {}
        CommandRtu::~CommandRtu() {}
        void CommandRtu::print() {
            std::cout << "CommandRtu: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            std::cout << this->fromAddr.getAddr() << std::endl;
            std::cout << this->toAddr.getAddr() << std::endl;
            std::cout << this->length << std::endl;
            std::cout << this->siteCode.getSiteCode() << std::endl;
            std::cout << this->dcCommand.getCommand() << std::endl;
            std::cout << this->acCommand.getCommand() << std::endl;
            printf("crc8: 0x%02x \n", this->crc8);
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class CommandRtuAck
        CommandRtuAck::CommandRtuAck() : stx(STX), etx(ETX), cmd(COMMAND_RTU_ACK), length(18) {}
        CommandRtuAck::~CommandRtuAck() {}
        void CommandRtuAck::print() {
            std::cout << "CommandRtu: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            std::cout << this->fromAddr.getAddr() << std::endl;
            std::cout << this->toAddr.getAddr() << std::endl;
            std::cout << this->length << std::endl;
            std::cout << this->siteCode.getSiteCode() << std::endl;
            std::cout << this->dcCommand.getCommand() << std::endl;
            std::cout << this->acCommand.getCommand() << std::endl;
            std::cout << this->result.getResult() << std::endl;
            printf("crc8: 0x%02x \n", this->crc8);
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class ClientInitReq
        ClientInitReq::ClientInitReq() : stx(STX), etx(ETX), cmd(CLIENT_INIT_REQ), length(2) {}
        ClientInitReq::~ClientInitReq() {}
        void ClientInitReq::print() {
            std::cout << "CommandRtu: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            std::cout << this->fromAddr.getAddr() << std::endl;
            std::cout << this->toAddr.getAddr() << std::endl;
            std::cout << this->length << std::endl;
            printf("crc8: 0x%02x \n", this->crc8);
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class ClientInitRes        
        ClientInitRes::ClientInitRes() : stx(STX), etx(ETX), cmd(CLIENT_INIT_RES), length(4) {}
        ClientInitRes::~ClientInitRes() {}
        void ClientInitRes::print() {
            std::cout << "CommandRtu: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            std::cout << this->fromAddr.getAddr() << std::endl;
            std::cout << this->toAddr.getAddr() << std::endl;
            std::cout << this->length << std::endl;
            std::cout << this->clientAddr.getAddr() << std::endl;
            printf("crc8: 0x%02x \n", this->crc8);
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class CommandClient        
        CommandClient::CommandClient() : stx(STX), etx(ETX), cmd(COMMAND_CLIENT), length(17) {}
        CommandClient::~CommandClient() {}
        void CommandClient::print() {
            std::cout << "CommandRtu: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            std::cout << this->fromAddr.getAddr() << std::endl;
            std::cout << this->toAddr.getAddr() << std::endl;
            std::cout << this->length << std::endl;
            std::cout << this->siteCode.getSiteCode() << std::endl;
            std::cout << this->dcCommand.getCommand() << std::endl;
            std::cout << this->acCommand.getCommand() << std::endl;
            printf("crc8: 0x%02x \n", this->crc8);
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class CommandClientAck        
        CommandClientAck::CommandClientAck() : stx(STX), etx(ETX), cmd(COMMAND_CLIENT_ACK), length(18) {}
        CommandClientAck::~CommandClientAck() {}
        void CommandClientAck::print() {
            std::cout << "CommandRtu: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            std::cout << this->fromAddr.getAddr() << std::endl;
            std::cout << this->toAddr.getAddr() << std::endl;
            std::cout << this->length << std::endl;
            std::cout << this->siteCode.getSiteCode() << std::endl;
            std::cout << this->dcCommand.getCommand() << std::endl;
            std::cout << this->acCommand.getCommand() << std::endl;
            std::cout << this->result.getResult() << std::endl;
            printf("crc8: 0x%02x \n", this->crc8);
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class SetupInfo
        SetupInfo::SetupInfo() : stx(STX), etx(ETX), cmd(SETUP_INFO), length(10) {}
        SetupInfo::~SetupInfo() {}
        void SetupInfo::print() {
            std::cout << "CommandRtu: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            std::cout << this->fromAddr.getAddr() << std::endl;
            std::cout << this->toAddr.getAddr() << std::endl;
            std::cout << this->length << std::endl;
            std::cout << this->action.getAction() << std::endl;
            std::cout << this->siteCode.getSiteCode() << std::endl;
            printf("crc8: 0x%02x \n", this->crc8);
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class SetupInfoAck
        SetupInfoAck::SetupInfoAck() : stx(STX), etx(ETX), cmd(SETUP_INFO_ACK), length(11) {}
        SetupInfoAck::~SetupInfoAck() {}
        void SetupInfoAck::print() {
            std::cout << "CommandRtu: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            std::cout << this->fromAddr.getAddr() << std::endl;
            std::cout << this->toAddr.getAddr() << std::endl;
            std::cout << this->length << std::endl;
            std::cout << this->action.getAction() << std::endl;
            std::cout << this->siteCode.getSiteCode() << std::endl;
            std::cout << this->result.getResult() << std::endl;
            printf("crc8: 0x%02x \n", this->crc8);
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class RtuStatusReq
        RtuStatusReq::RtuStatusReq() : stx(STX), etx(ETX), cmd(RTU_STATUS_REQ), length(2) {}
        RtuStatusReq::~RtuStatusReq() {}
        void RtuStatusReq::print() {
            std::cout << "CommandRtu: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            std::cout << this->fromAddr.getAddr() << std::endl;
            std::cout << this->toAddr.getAddr() << std::endl;
            std::cout << this->length << std::endl;
            printf("crc8: 0x%02x \n", this->crc8);
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
        // Class RtuStatusRes
        RtuStatusRes::RtuStatusRes() : stx(STX), etx(ETX), cmd(RTU_STATUS_RES) {}
        RtuStatusRes::~RtuStatusRes() {}
        void RtuStatusRes::print() {
            std::cout << "CommandRtu: " << std::endl;
            printf("stx: 0x%02x \n", this->stx);
            printf("cmd: 0x%02x \n", this->cmd);
            std::cout << this->fromAddr.getAddr() << std::endl;
            std::cout << this->toAddr.getAddr() << std::endl;
            std::cout << this->length << std::endl;
            std::cout << this->count << std::endl;
            printf("crc8: 0x%02x \n", this->crc8);
            printf("etx: 0x%02x \n", this->etx);
        }
        //////////////////////////////////////////////
    }
}