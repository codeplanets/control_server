#pragma once

#include "uncopyable.h"

namespace core {
    namespace common {
        class CriticalSection : private Uncopyable {
        public:
            class Owner {
            private:
                CriticalSection& m_Instance;
            public:
                Owner(CriticalSection& obj);
                ~Owner(void);
            };
        private:
            void* m_criticalSection;

        public:
            CriticalSection(void);
            ~CriticalSection(void);
            void enter(void);
            void leave(void);

        private:
            void* initializeCriticalSection(void);
            void deleteCriticalSection(void* hCS);
            void enterCriticalSection(void* hCS);
            void leaveCriticalSection(void* hCS);

        };
    }
}