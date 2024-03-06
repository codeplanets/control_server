#include "packetizer.h"

#include "message.h"

namespace core {
    namespace server {
        Packetizer::Packetizer() {

        }

        Packetizer::~Packetizer() {

        }

        /**
         * Message constructor
        */
        core::formatter::InitRes Packetizer::getMessage_initres(char cmd) {
            return core::formatter::InitRes();
        }
        core::formatter::HeartBeatAck Packetizer::getMessage_heartbeatack(char cmd) {
            return core::formatter::HeartBeatAck();
        }
        core::formatter::CommandRtu Packetizer::getMessage_commandrtu(char cmd) {
            return core::formatter::CommandRtu();
        }
    }
}