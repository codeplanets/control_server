#pragma once


#include <iostream>
#include <string>
#include <regex>

#define MAX_TASK_NUM	1
#define MAX_EXTEND_NUM	1
#define MAX_RAW_BUFF    4096
#define BUFF_SIZE       40960
#define STX_V       0xFA
#define ETX_V       0xF5
#define STX (unsigned char)0xFA     /* RTU DATA FORMAT ==> STX */
#define ETX (unsigned char)0xF5     /* RTU DATA FORMAT ==> ETX */

// Message Types --------------------------------
#define INIT_REQ (unsigned char)0x01
#define INIT_RES (unsigned char)0x02
#define HEART_BEAT (unsigned char)0x03
#define HEART_BEAT_ACK (unsigned char)0x04
#define COMMAND_RTU (unsigned char)0x05
#define COMMAND_RTU_ACK (unsigned char)0x06
#define CLIENT_INIT_REQ (unsigned char)0x11
#define CLIENT_INIT_RES (unsigned char)0x12
#define COMMAND_CLIENT (unsigned char)0x15
#define COMMAND_CLIENT_ACK (unsigned char)0x16
#define SETUP_INFO (unsigned char)0x17
#define SETUP_INFO_ACK (unsigned char)0x18
#define RTU_STATUS_REQ (unsigned char)0x19
#define RTU_STATUS_RES (unsigned char)0x1F

typedef int SOCKET;

namespace core {
    namespace common {
        void sleep(unsigned int dwMilliSec);
    }
}