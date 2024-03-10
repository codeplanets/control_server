#pragma once

#include <iostream>
#include <string>
#include <regex>

typedef int SOCKET;
typedef u_char DATA;

#define MAX_TASK_NUM	1
#define MAX_EXTEND_NUM	1
#define MAX_RAW_BUFF    4096
#define BUFF_SIZE       40960
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

const std::string rtu_mq_name = "/rtu.";
const std::string server_mq_name = "/server.";
const std::string client_mq_name = "/client.";

namespace core {
    namespace common {
        void sleep(unsigned int dwMilliSec);
        void print_hex(DATA *buf, int size);
        u_short convert_be_to_le(DATA* be, int size);
        DATA calcCRC(DATA *buf, int size);
    }
}

#include <syslog.h>