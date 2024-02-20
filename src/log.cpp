#include "log.h"

namespace core {
    namespace system {
        
        void InitLog(std::string strFilePath, bool bEncrypt, unsigned int dwInputFlag)
        {
            
        }
        //////////////////////////////////////////////////////////////////////////	
        void InitLog(const ST_LOG_INIT_PARAM& stParam)
        {
        }

        void logDebug(const char* pszFormat, ...) {
            va_list list;
            va_start(list, pszFormat);
            va_end(list);
        }

        void logInfo(const char* pszFormat, ...) {
            va_list list;
            va_start(list, pszFormat);
            va_end(list);
        }

        void logWarn(const char* pszFormat, ...){
            va_list list;
            va_start(list, pszFormat);
            va_end(list);
        }

        void logError(const char* pszFormat, ...){
            va_list list;
            va_start(list, pszFormat);
            va_end(list);
        }

        void logTrace(const char* pszFormat, ...){
            va_list list;
            va_start(list, pszFormat);
            va_end(list);
        }

        ST_LOG_CONTEXT::ST_LOG_CONTEXT(void)
			: fileCS(NULL), processID(0), inputFlag(LOG_DEFAULT), outputFlag(LOG_OUTPUT_CONSOLE), maxFileSize(10 * 1000 * 1000), maxFileCount(3)
			, outputFile(), id(), fpCustomOutput(NULL)
		{
		}

        //////////////////////////////////////////////////////////////////////////
		ST_LOG_CONTEXT::~ST_LOG_CONTEXT(void)
		{
		}

		//////////////////////////////////////////////////////////////////////////
		GlobalLogContext::GlobalLogContext(void)
			: ST_LOG_CONTEXT()
		{
		}

		//////////////////////////////////////////////////////////////////////////
		GlobalLogContext::~GlobalLogContext(void)
		{
		}

		//////////////////////////////////////////////////////////////////////////
		void Log_FormatV(unsigned int unInputType, const char* pszFormat, va_list vaList)
		{
		}

    }
}