#pragma once

#include "common.h"

namespace core {

    struct CmdLog {
    public:
        CmdLog() : ack(false), ackResult("N"), resultCode(NOT_ACK) {}

        std::string siteCode;
        std::string date;
        std::string time;

        bool ack;
        std::string ackDate;
        std::string ackTime;
        std::string ackResult;
        int resultCode;
    };
}