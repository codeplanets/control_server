#include "db.h"

namespace core {
    namespace system {
        Database::Database() : m_pConn(NULL) {

        }
        Database::~Database() {
            db_close();
        }

        MYSQL* Database::getConnection() {
            return m_pConn;
        }

        ECODE Database::db_init() {
            ConfigParser parser = ConfigParser(config_file);
            string server = parser.aConfig<string>("database", "address");
            unsigned int port = parser.aConfig<unsigned int>("database", "port");
            string user = parser.aConfig<string>("database", "id");
            string password = parser.aConfig<string>("database", "password");
            string database = parser.aConfig<string>("database", "database");

            return db_init(server.c_str(), port, user.c_str(), password.c_str(), database.c_str());
        }

        ECODE Database::db_init(const char* server, unsigned int port, const char* user, const char* password, const char* database) {
            m_pConn = mysql_init(NULL);
            if (!m_pConn) {
                syslog(LOG_ERR, "error");
                return EC_FAILURE;
            } else {
                syslog(LOG_DEBUG, "Init Database....");
            }

            m_pConn = mysql_real_connect(m_pConn, server, user, password, database, port, NULL, 0);
            if (m_pConn) {
                syslog(LOG_DEBUG, "Success connect to database.");
            } else {
                syslog(LOG_ERR, "Connect error!");
                return EC_FAILURE;
            }

            return EC_SUCCESS;
        }

        void Database::db_close() {
            if (m_pConn) {
                mysql_close(m_pConn);
                m_pConn = NULL;
            }
        }
        ECODE Database::db_query(const char* query) {
            if (m_pConn) {
                if (mysql_query(m_pConn, query)) {
                    return EC_FAILURE;
                } else {
                    return EC_SUCCESS;
                }
            }
            return EC_FAILURE;
        }
        ECODE Database::db_query(const char* query, MYSQL_RES** pRes) {
            if (m_pConn) {
                if (mysql_query(m_pConn, query)) {
                    return EC_FAILURE;
                } else {
                    *pRes = mysql_store_result(m_pConn);
                    return EC_SUCCESS;
                }
            }
            return EC_FAILURE;
        }
        ECODE Database::db_query(const char* query, MYSQL_RES** pRes, MYSQL_ROW* pRow) {
            if (m_pConn) {
                if (mysql_query(m_pConn, query)) {
                    return EC_FAILURE;
                } else {
                    *pRes = mysql_store_result(m_pConn);
                    *pRow = mysql_fetch_row(*pRes);
                    return EC_SUCCESS;
                }
            }
            return EC_FAILURE;
        }
        MYSQL_RES* Database::db_get_result(const char* query) {
            if (m_pConn) {
                if (mysql_query(m_pConn, query)) {
                    return NULL;
                } else {
                    return mysql_store_result(m_pConn);
                }
            }
            return NULL;
        }
        MYSQL_ROW Database::db_fetch_row(MYSQL_RES* pRes) {
            if (pRes) {
                return mysql_fetch_row(pRes);
            }
            return NULL;
        }
        void Database::db_free_result(MYSQL_RES* pRes) {
            if (pRes) {
                mysql_free_result(pRes);
            }
        }
    }
}