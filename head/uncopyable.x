#pragma once

namespace core {
    namespace common {
        class Uncopyable {
            Uncopyable(const Uncopyable& other);
            Uncopyable& operator=(const Uncopyable& other);
        public:
            Uncopyable(void) {}
        };
    }
}