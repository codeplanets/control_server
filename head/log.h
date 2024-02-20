#pragma once

#include <stdarg.h>
#include <iostream>

namespace core {
    namespace system {
        enum E_LOG_INPUT_TYPE {
            LOG_DEBUG   = 0x01 << 0,
            LOG_INFO    = 0x01 << 1,
            LOG_WARN    = 0x01 << 2,
            LOG_ERROR   = 0x01 << 3,
            LOG_TRACE   = 0x01 << 4,

            LOG_DEFAULT = LOG_INFO|LOG_WARN|LOG_ERROR|LOG_TRACE,
            LOG_ALL	    = LOG_INFO|LOG_WARN|LOG_ERROR|LOG_TRACE|LOG_DEBUG,
        };
        enum E_LOG_OUTPUT_TYPE {
            LOG_OUTPUT_FILE     = 0x01 << 0,
            LOG_OUTPUT_CONSOLE  = 0x01 << 1,

            LOG_OUTPUT_DEFAULT		= LOG_OUTPUT_FILE|LOG_OUTPUT_CONSOLE,
        };

        struct ST_LOG_INIT_PARAM {
            std::string     logPath;
            std::string     modulePath;
            unsigned int    inputFlag;
            unsigned int    outputFlag;
            unsigned int    maxFileSize;
            unsigned int    maxFileCount;
            void (*fpCustomOutput)(const char* pszPrefix, const char* pszMsg);

            ST_LOG_INIT_PARAM(void)
                : inputFlag(LOG_DEFAULT), outputFlag(LOG_OUTPUT_DEFAULT), maxFileSize(10 * 1000 * 1000), maxFileCount(10)
                , fpCustomOutput(NULL)	{}
        };

        struct ST_SYSTEMTIME {
            unsigned short year;
            unsigned short month;
            unsigned short dayOfWeek;
            unsigned short day;
            unsigned short hour;
            unsigned short minute;
            unsigned short second;
            unsigned short milliseconds;
        };

        struct ST_LOG_CONTEXT
		{
			void*           fileCS;
			unsigned int    processID;
			unsigned int    inputFlag;
			unsigned int    outputFlag;
			unsigned int    maxFileSize;
			unsigned int    maxFileCount;
			std::string     outputFile;
			std::string     id;
			ST_SYSTEMTIME	lastTime;
			ST_SYSTEMTIME	lastCheckTime;
			void(*fpCustomOutput)(const char* pszPrefix, const char* pszMsg);

			ST_LOG_CONTEXT(void);
			~ST_LOG_CONTEXT(void);
		};

        
        void initLog(std::string strPath, bool bEncrypt = false, unsigned int unInputFlag = LOG_INFO|LOG_WARN|LOG_ERROR);
        void initLog(const ST_LOG_INIT_PARAM& stParam);

        void logDebug(const char* pszFormat, ...);
        void logInfo(const char* pszFormat, ...);
        void logWarn(const char* pszFormat, ...);
        void logError(const char* pszFormat, ...);
        void logTrace(const char* pszFormat, ...);

        
		class GlobalLogContext : public ST_LOG_CONTEXT
		{
			GlobalLogContext(void);
			~GlobalLogContext(void);

		public:
			static GlobalLogContext* GetInstance(void)
			{
				static GlobalLogContext instance;
				return &instance;
			}
		};

		inline GlobalLogContext* GlobalLog(void)		{	return GlobalLogContext::GetInstance();	}

		void logFormatV(unsigned int unInputType, const char* pszFormat, va_list vaList);
    }
}