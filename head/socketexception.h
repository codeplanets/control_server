#pragma once

#include <string>
#include "errorcode.h"

namespace core {
    class SocketException {
    private:
        std::string m_s;
        ECODE m_c;

    public:
        SocketException(ECODE c, std::string s)
         : m_c(c) 
         , m_s(s) {}
        ~SocketException() {}
        ECODE code() {return m_c;}
        std::string description() {return m_s;}
    };
}
