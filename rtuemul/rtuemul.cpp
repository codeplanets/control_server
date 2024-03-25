#include "rtuemul.h"

#define PORT 5900

using namespace std;

const unsigned char hello[] = {0xFA, 0x01, 0x00, 0x00, 0x30, 0x01, 0x07, 0x00, 0x31, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x7D, 0xF5};
const unsigned char heartbeat[] = {0xFA, 0x03, 0x10, 0x01, 0x30, 0x01, 0x00, 0x00, 0x59, 0xF5};
const unsigned char commandAck[] = {0xFA, 0x06, 0x10, 0x01, 0x20, 0x01, 0x10, 0x00, 0x31, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x59, 0x4E, 0x59, 0x4E, 0x59, 0x4E, 0x59, 0x4E, 0x01, 0x6D, 0xF5};
const unsigned char bhcmdack[] = {0xFA, 0x03, 0x10, 0x01, 0x30, 0x01, 0x00, 0x00, 0x59, 0xF5, 0xFA, 0x06, 0x10, 0x01, 0x20, 0x01, 0x10, 0x00, 0x31, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x59, 0x4E, 0x59, 0x4E, 0x59, 0x4E, 0x59, 0x4E, 0x01, 0x6D, 0xF5};

SiteCode scode;
Address rtuAddr;
Address cmdAddr;
Command dcCommand;
Command acCommand;

static int sock = 0, valread;
struct sockaddr_in serv_addr;

DATA sendbuf[MAX_RAW_BUFF] = {0, };
DATA buffer[MAX_RAW_BUFF] = {0, };

const int HEARTBEAT_TIME = 30;

void heartbeat_handler(int sig) {
    if (sig == SIGALRM) {
        // Send message to the server
        u_short addr = rtuAddr.getAddr();
        printf("addr : 0x%02X \n", addr);
        int sendByte = reqMessage(sendbuf, HEART_BEAT, addr);
        ssize_t len = send(sock, sendbuf, sendByte, 0);
        cout << len << endl;
    }
}

void setTimer() {
    struct sigaction action;
    action.sa_handler = heartbeat_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGALRM, &action, NULL);
    alarm(HEARTBEAT_TIME);
}

int main(int argc, char const *argv[]) {
    int msgSize = 0;
    int sendByte = 0;

    if (argc > 1) {
        scode.setSiteCode(argv[1]);
    } else return 0;

    // Create socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cerr << "Socket creation error" << endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        cerr << "Invalid address/ Address not supported" << endl;
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "Connection Failed" << endl;
        return -1;
    }

    // Send message to the server
    sendByte = reqMessage(sendbuf, INIT_REQ, 0x0000);
    send(sock, sendbuf, sendByte, 0);

    while(1) {
        // Receive message from the server
        valread = read(sock, buffer, MAX_RAW_BUFF);
        cout << valread << endl;
        if (valread > 0) {
            if (buffer[0] == STX) {
                if (buffer[1] == INIT_RES) {
                    setTimer();
                    InitRes msg;
                    memcpy((void*)&msg, buffer, valread);
                    msg.print();
                    rtuAddr = msg.rtuAddr;
                    printf("0x%02X \n", rtuAddr.getAddr());
                } else if (buffer[1] == HEART_BEAT_ACK) {
                    HeartBeatAck msg;
                    memcpy((void*)&msg, buffer, valread);
                    msg.print();
                    alarm(HEARTBEAT_TIME);
                } else if (buffer[1] == COMMAND_RTU) {
                    CommandRtu msg;
                    memcpy((void*)&msg, buffer, valread);
                    msg.print();
                    cmdAddr = msg.toAddr;
                    dcCommand = msg.dcCommand;
                    acCommand = msg.acCommand;
                    sendByte = reqMessage(sendbuf, COMMAND_RTU_ACK, rtuAddr.getAddr());
                    send(sock, sendbuf, sendByte, 0);

                } else {
                    print_hex(buffer, valread);
                }
            }
        } else {
            u_short addr = rtuAddr.getAddr();
            printf("addr : 0x%02X \n", addr);
            int sendByte = reqMessage(sendbuf, HEART_BEAT, addr);
            send(sock, sendbuf, sendByte, 0);
        }
    }

    return 0;
}

int reqMessage(DATA* buf, DATA cmd, u_short addr) {
    // INIT_RES, HEART_BEAT_ACK, COMMAND_RTU
    if (cmd == INIT_REQ) {
        InitReq msg;
        cout << "reqMessage::INIT_REQ" << endl;

        // Packetizer
        msg.fromAddr.setAddr(addr);
        msg.toAddr.setAddr(SERVER_ADDRESS);
        msg.siteCode = scode;
        msg.crc8.setCRC8(calcCRC((DATA*)&msg, sizeof(msg)));
        msg.print();

        memcpy(buf, (char*)&msg, sizeof(msg));
        print_hex(buf, sizeof(msg));
        return sizeof(msg);

    } else if (cmd == HEART_BEAT) {
        HeartBeat msg;
        cout << "reqMessage::HEART_BEAT" << endl;
        
        msg.fromAddr.setAddr(addr);
        msg.toAddr.setAddr(SERVER_ADDRESS);
        msg.crc8.setCRC8(calcCRC((DATA*)&msg, sizeof(msg)));
        msg.print();

        memcpy(buf, (char*)&msg, sizeof(msg));
        print_hex(buf, sizeof(msg));
        return sizeof(msg);

    } else if (cmd == COMMAND_RTU_ACK) {
        CommandRtuAck msg;
        cout << "reqMessage::COMMAND_RTU_ACK" << endl;

        msg.fromAddr.setAddr(addr);
        msg.toAddr = cmdAddr;
        msg.siteCode = scode;
        msg.dcCommand = dcCommand;
        msg.acCommand = acCommand;
        msg.result.setResult(COMMAND_RESULT_OK);
        msg.crc8.setCRC8(calcCRC((DATA*)&msg, sizeof(msg)));
        msg.print();

        memcpy(buf, (char*)&msg, sizeof(msg));
        print_hex(buf, sizeof(msg));
        return sizeof(msg);

    } else {
        cout << "Unknown message type." << endl;
    }
    return 0;
}

u_short convert_be_to_le(DATA* be, int size) {
    if (size == 2) {
        u_short x = 0;
        memcpy(&x, be, size);
        x = ((x & 0xFF00) >> 8) | ((x & 0x00FF) << 8);
        return x;
    }
    return 0;
}
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
void Address::setAddr(string addr, char section) {   // DB에서 읽어온 주소 변환
    u_short num = stoi(addr);
    setAddr(num, section);
}
u_short Address::getAddr() {
    return convert_be_to_le(addr, 2);
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
string Result::getStrResult() {
    return this->result == 0x01? "Y" : "N";
}
void Result::setResult(char result) {
    this->result = result;            
}
//////////////////////////////////////////////
// Class InitReq
InitReq::InitReq() : stx(STX), cmd(INIT_REQ), length(7), etx(ETX) {}
InitReq::~InitReq() {}
void InitReq::print() {
    printf("InitReq::print() "); printf("\n");
    printf("cmd: 0x%02X", this->cmd);printf("\n");
    printf("frm: 0x%04X", this->fromAddr.getAddr());printf("\n");
    printf("to : 0x%04X", this->toAddr.getAddr());printf("\n");
    printf("len: %u", this->length);printf("\n");
    printf("scd: %s", this->siteCode.getSiteCode());printf("\n");
    printf("crc: 0x%02X", this->crc8.getCRC8());printf("\n");
    printf("etx: 0x%02X", this->etx);printf("\n");
}
//////////////////////////////////////////////
// Class InitRes
InitRes::InitRes() : stx(STX), cmd(INIT_RES), length(9), etx(ETX) {}
InitRes::~InitRes() {}
void InitRes::print() {
    printf("InitRes::print() ");printf("\n");
    printf("stx: 0x%02X", this->stx);printf("\n");
    printf("cmd: 0x%02X", this->cmd);printf("\n");
    printf("frm: 0x%04X", this->fromAddr.getAddr());printf("\n");
    printf("to : 0x%04X", this->toAddr.getAddr());printf("\n");
    printf("len: %u", this->length);printf("\n");
    printf("scd: %s", this->siteCode.getSiteCode());printf("\n");
    printf("rtu: 0x%04X", this->rtuAddr.getAddr());printf("\n");
    printf("crc: 0x%02X", this->crc8.getCRC8());printf("\n");
    printf("etx: 0x%02X", this->etx);printf("\n");
}
//////////////////////////////////////////////
// Class HeartBeat
HeartBeat::HeartBeat() : stx(STX), cmd(HEART_BEAT), length(0), etx(ETX) {}
HeartBeat::~HeartBeat() {}
void HeartBeat::print() {
    printf("HeartBeat::print() ");printf("\n");
    printf("stx: 0x%02X", this->stx);printf("\n");
    printf("cmd: 0x%02X", this->cmd);printf("\n");
    printf("frm: 0x%04X", this->fromAddr.getAddr());printf("\n");
    printf("to : 0x%04X", this->toAddr.getAddr());printf("\n");
    printf("len: %u", this->length);printf("\n");
    printf("crc: 0x%02X", this->crc8.getCRC8());printf("\n");
    printf("etx: 0x%02X", this->etx);printf("\n");
}
//////////////////////////////////////////////
// Class HeartBeatAck
HeartBeatAck::HeartBeatAck() : stx(STX), cmd(HEART_BEAT_ACK), length(0), etx(ETX) {}
HeartBeatAck::~HeartBeatAck() {}
void HeartBeatAck::print() {
    printf("HeartBeatAck::print() ");printf("\n");
    printf("stx: 0x%02X", this->stx);printf("\n");
    printf("cmd: 0x%02X", this->cmd);printf("\n");
    printf("frm: 0x%04X", this->fromAddr.getAddr());printf("\n");
    printf("to : 0x%04X", this->toAddr.getAddr());printf("\n");
    printf("len: %u", this->length);printf("\n");
    printf("crc: 0x%02X", this->crc8.getCRC8());printf("\n");
    printf("etx: 0x%02X", this->etx);printf("\n");
}
//////////////////////////////////////////////
// Class CommandRtu
CommandRtu::CommandRtu() : stx(STX), cmd(COMMAND_RTU), length(15), etx(ETX) {}
CommandRtu::~CommandRtu() {}
void CommandRtu::print() {
    printf("CommandRtu::print() ");printf("\n");
    printf("stx: 0x%02X", this->stx);printf("\n");
    printf("cmd: 0x%02X", this->cmd);printf("\n");
    printf("frm: 0x%04X", this->fromAddr.getAddr());printf("\n");
    printf("to : 0x%04X", this->toAddr.getAddr());printf("\n");
    printf("len: %u", this->length);printf("\n");
    printf("scd: %s", this->siteCode.getSiteCode());printf("\n");
    printf("dc : %s", this->dcCommand.getCommand());printf("\n");
    printf("ac : %s", this->acCommand.getCommand());printf("\n");
    printf("crc: 0x%02X", this->crc8.getCRC8());printf("\n");
    printf("etx: 0x%02X", this->etx);printf("\n");
}
//////////////////////////////////////////////
// Class CommandRtuAck
CommandRtuAck::CommandRtuAck() : stx(STX), cmd(COMMAND_RTU_ACK), length(16), etx(ETX) {}
CommandRtuAck::~CommandRtuAck() {}
void CommandRtuAck::print() {
    printf("CommandRtuAck::print() ");printf("\n");
    printf("stx: 0x%02X", this->stx);printf("\n");
    printf("cmd: 0x%02X", this->cmd);printf("\n");
    printf("frm: 0x%04X", this->fromAddr.getAddr());printf("\n");
    printf("to : 0x%04X", this->toAddr.getAddr());printf("\n");
    printf("len: %u", this->length);printf("\n");
    printf("scd: %s", this->siteCode.getSiteCode());printf("\n");
    printf("dc : %s", this->dcCommand.getCommand());printf("\n");
    printf("ac : %s", this->acCommand.getCommand());printf("\n");
    printf("rst: %0X", this->result.getRawResult());printf("\n");
    printf("crc: 0x%02X", this->crc8.getCRC8());printf("\n");
    printf("etx: 0x%02X", this->etx);printf("\n");
}

DATA calcCRC(DATA *buf, int size) {
    if (size < 10) {
        return 0x00;
    }

    DATA CRC = 0x00;
    int len = size - 2;

    for (int i = 0; i < len; i++) {
        CRC ^= *(buf + i);
    }
    CRC = CRC & 0x7F;

    return CRC;
}

void print_hex(unsigned char *buf, int size) {
    int index;
    for (index = 0; index < size; index++) {
        if (index % 16 == 0) cout << endl;
        printf("0x%02X ", buf[index]);
    }
    cout << endl;
}
