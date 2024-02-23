#pragma once

#include <queue>

#include "criticalsection.h"

namespace core {
    using namespace core::common;
    
    template<typename T>
    class SafeQueue : private Uncopyable {
        CriticalSection m_csData;
        std::queue<T> m_queData;

    public:
        SafeQueue(void) : m_csData(), m_queData()		{}
        ~SafeQueue(void)								{}

        void Clear(void) {
            CriticalSection::Owner Lock(m_csData);
            while(!m_queData.empty()) {
                m_queData.pop();
            }
        }

        bool IsEmpty(void) {
            CriticalSection::Owner Lock(m_csData);
            return m_queData.empty();
        }

        size_t Count(void) {
            CriticalSection::Owner Lock(m_csData);
            return m_queData.size();
        }

        ECODE Push(const T& data) {
            CriticalSection::Owner Lock(m_csData);
            m_queData.push(data);
            return EC_SUCCESS;
        }

        ECODE Pop(T& outItem) {
            CriticalSection::Owner Lock(m_csData);
            if( m_queData.empty() ) {
                return EC_NO_DATA;
            }

            outItem = m_queData.front();
            m_queData.pop();
            return EC_SUCCESS;
        }
    };
}