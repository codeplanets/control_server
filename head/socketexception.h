#pragma once

#include <string>
#include "errorcode.h"

namespace core {
    class SocketException {
    private:
        ECODE m_c;
        std::string m_s;

    public:
        SocketException(ECODE c, std::string s)
         : m_c(c) 
         , m_s(s) {}
        ~SocketException() {}
        ECODE code() {return m_c;}
        std::string description() {return m_s;}
    };
}
