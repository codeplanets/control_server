#pragma once

#include <vector>
#include <string>

#include "errorcode.h"
#include "safequeue.h"

namespace core {

    class ThreadPool;
    namespace internal {
        
        struct ST_THREADPOOL_DATA {
            std::string strName;
            void* thread;
            void* event;
            bool* pbEscapeFlag;

            ThreadPool* pOwner;

            int (*pfThreadFunc)(void *pContext);
            void* pContext;

            ST_THREADPOOL_DATA(void)
            : strName(), thread(NULL), event(NULL)
            , pbEscapeFlag(NULL), pOwner(NULL)
            , pfThreadFunc(NULL), pContext(NULL) {

            }
        };
    }

    typedef internal::ST_THREADPOOL_DATA* THREADPOOL;
    
    enum E_THREAD_PRIORITY
    {
        THREAD_PRIORITY_ERROR		= -100,
        THREAD_PRIORITY_BELOW_LOW	= -2,
        THREAD_PRIORITY_LOW			= -1,
        THREAD_PRIORITY_MIDDLE		= 0,
        THREAD_PRIORITY_HIGH		= 1,
        THREAD_PRIORITY_ABOVE_HIGH	= 2,
    };

    class ThreadPool {
        std::vector<internal::ST_THREADPOOL_DATA> m_vecInstance;
        SafeQueue<internal::ST_THREADPOOL_DATA> m_qReady;

        bool m_bEscapeFlag;
        bool m_bReserved[7];
    public:
        ThreadPool();
        virtual ~ThreadPool();

        ECODE init(size_t poolCount, E_THREAD_PRIORITY nPriority = THREAD_PRIORITY_MIDDLE);
        void destroy(void);
        
        ECODE createThread(int(*pfEntry)(void* pContext), void* pContext);
    private:
        friend int threadPoolWorker(void* pContext);
        void returnThread(THREADPOOL thread);
    };
}