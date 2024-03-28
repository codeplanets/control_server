#pragma once
#pragma pack(1)

#include <iostream>
#include <string>
#include <regex>
#include <cassert>
#include <sys/mman.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <signal.h>

typedef int SOCKET;
typedef u_char DATA;

#define STX_V       0xFA
#define ETX_V       0xF5
#define STX (DATA)0xFA     /* RTU DATA FORMAT ==> STX */
#define ETX (DATA)0xF5     /* RTU DATA FORMAT ==> ETX */

#define DEFAULT_ADDRESS (DATA)0x00
#define RTU_ADDRESS (DATA)0x10
#define CLIENT_ADDRESS (DATA)0x20
#define SERVER_ADDRESS (u_short)0x3001

// Message Types --------------------------------
#define INIT_REQ (DATA)0x01
#define INIT_RES (DATA)0x02
#define HEART_BEAT (DATA)0x03
#define HEART_BEAT_ACK (DATA)0x04
#define COMMAND_RTU (DATA)0x05
#define COMMAND_RTU_ACK (DATA)0x06
#define CLIENT_INIT_REQ (DATA)0x11
#define CLIENT_INIT_RES (DATA)0x12
#define COMMAND_CLIENT (DATA)0x15
#define COMMAND_CLIENT_ACK (DATA)0x16
#define SETUP_INFO (DATA)0x17
#define SETUP_INFO_ACK (DATA)0x18
#define RTU_STATUS_REQ (DATA)0x19
#define RTU_STATUS_RES (DATA)0x1F

#define COMMAND_RESULT_OK (DATA)0x01
#define COMMAND_RESULT_NOT_FOUND (DATA)0x02
#define COMMAND_RESULT_NOT_CONNECT (DATA)0x03
#define COMMAND_RESULT_NOT_ACK (DATA)0x04

#define ACTION_RESULT_OK (DATA)0x01
#define ACTION_RESULT_FAIL (DATA)0x02

#define STATUS_CONNECTED (DATA)0x01
#define STATUS_DISCONNECTED (DATA)0x02

const int MAX_RAW_BUFF = 8192;

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
private:
    char result;
public:
    char getRawResult();
    std::string getStrResult();
    void setResult(char);
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
    Result result;
    CRC crc8;
    DATA etx;
};
u_short convert_be_to_le(DATA* be, int size);
int reqMessage(DATA* buf, DATA cmd, u_short addr);
DATA calcCRC(DATA *buf, int size);
void print_hex(unsigned char *buf, int size);