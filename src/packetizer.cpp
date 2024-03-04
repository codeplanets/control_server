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
        core::formatter::Message Packetizer::getMessage(char cmd) {
            if (cmd == INIT_RES) {
                return core::formatter::InitRes();
            } else if (cmd == HEART_BEAT_ACK) {
                return core::formatter::HeartBeatAck();
            } else if (cmd == COMMAND_RTU) {
                return core::formatter::CommandRtu();
            } else {
                return core::formatter::Message();
            }
        }
    }
}