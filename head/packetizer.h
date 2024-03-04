#pragma once

#include "message.h"

namespace core {
    namespace server {
        class Packetizer {
        public:
            Packetizer();
            virtual ~Packetizer();

            static core::formatter::Message getMessage(char cmd);
        };
    }
}
