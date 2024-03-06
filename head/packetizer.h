#pragma once

#include "message.h"

namespace core {
    namespace server {
        class Packetizer {
        public:
            Packetizer();
            virtual ~Packetizer();

            static core::formatter::InitRes getMessage_initres(char cmd);
            static core::formatter::HeartBeatAck getMessage_heartbeatack(char cmd);
            static core::formatter::CommandRtu getMessage_commandrtu(char cmd);
        };
    }
}
