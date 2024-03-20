#pragma once

#include <iostream>
#include <string>
#include <regex>
#include <cassert>

typedef int SOCKET;
typedef u_char DATA;

#define MAX_TASK_NUM	1
#define MAX_EXTEND_NUM	1
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

#define ACTION_RESULT_OK (DATA)0x01
#define ACTION_RESULT_FAIL (DATA)0x02

#define STATUS_CONNECTED (DATA)0x01
#define STATUS_DISCONNECTED (DATA)0x02

const std::string RTU_MQ_NAME = "/rtu.";
const std::string CLIENT_MQ_NAME = "/client.";

const int MAX_RAW_BUFF = 4096;

const bool test = false;
const int MAX_POOL = 255;
const int LISTEN_BACKLOG = 5;
const u_int WAITING_SEC = 60;
const u_int CMD_WAITING_SEC = 600;
const long MQ_MAXMSG = 10;
const long MQ_MSGSIZE = 2048;

const std::string NOT_FOUND = "NONE";
const std::string RTU_DATA = "./data/rtu.data";
const std::string CLIENT_DATA = "./data/cmd.data";

const int CONTROL_OK = 1;
const int SITE_NOT_FOUND = 2;
const int NOT_CONNECT = 3;
const int NOT_ACK = 4;

namespace core {
    namespace common {
        typedef struct mq_rtu_mapper {
            pid_t pid;
            u_short addr;

            bool operator < (const mq_rtu_mapper &var) const {
                if (pid == var.pid) return addr < var.addr;
                return pid > var.pid;
            }
        } MAPPER;

        // static MAPPER mapper_list[MAX_POOL] = {0, };

        void sleep(unsigned int dwMilliSec);
        void print_hex(DATA *buf, int size);
        u_short convert_be_to_le(DATA* be, int size);
        DATA calcCRC(DATA *buf, int size);
        bool checkCRC(DATA *buf, int size, DATA crc);
    }
}

#include <syslog.h>
/**
 * /etc/rsyslog.conf
 * user.*   /var/log/rsyslog
 * /var/log/rsyslog {
 *      daily
 *      copytruncate
 *      notifempty
 *      create 0666 rnsea rnsea
 *      dateext
 *      rotate 60
 * }
 * $ touch /var/log/rsyslog
 * $ chmod 0666 /var/log/rsyslog
 * $ logrotate -f /etc/logrotate.conf
*/

/**
 * 현재 사용 중인 Shared Memory의 크기 확인 (프로그램이 동작 중 이므로 사이즈가 점차 증가하고 있습니다.)
 * $ ls -ash /dev/shm
*/
