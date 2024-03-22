#pragma once

#include <mysql/mysql.h>

#include "common.h"
#include "errorcode.h"

#include "configparser.h"

using namespace std;

namespace core {
    namespace system {
        class Database {
        private:
            MYSQL* m_pConn;

        public:
            Database();
            ~Database();

            MYSQL* getConnection();

            ECODE db_init();
            ECODE db_init(const char* server, unsigned int port, const char* user, const char* password, const char* database);
            void db_close();

            ECODE db_query(const char* query);
            ECODE db_query(const char* query, MYSQL_RES** result);
            ECODE db_query(const char* query, MYSQL_RES** result, MYSQL_ROW* row);

            MYSQL_RES* db_get_result(const char* query);
            MYSQL_ROW db_fetch_row(MYSQL_RES* pRes);
            void db_free_result(MYSQL_RES* pRes);
        };
    }
}

/** RSite Table Structure
 * +----------+----------+--------+-------+
 * | SiteCode | SiteName | SiteID | Basin |
 * +----------+----------+--------+-------+
 * | 1000001  | Site01   |      1 |     1 |
 * | 1000002  | Site02   |      2 |     1 |
 * | 1000003  | Site03   |      3 |     1 |
 * | 1000004  | Site04   |      4 |     1 |
 * | 1000005  | Site05   |      5 |     1 |
 * | 1000006  | Site06   |      6 |     1 |
 * | 1000007  | Site07   |      7 |     1 |
 * | 1000008  | Site08   |      8 |     1 |
 * | 1000009  | Site09   |      9 |     1 |
 * | 1000010  | Site10   |     10 |     1 |
 * | 2000001  | Site11   |     11 |     2 |
 * | 2000002  | Site12   |     12 |     2 |
 * | 2000003  | Site13   |     13 |     2 |
 * | 2000004  | Site14   |     14 |     2 |
 * | 2000005  | Site15   |     15 |     2 |
 * | 3000001  | Site16   |     16 |     3 |
 * | 3000002  | Site17   |     17 |     3 |
 * | 3000003  | Site18   |     18 |     3 |
 * | 4000001  | Site19   |     19 |     4 |
 * | 4000002  | Site20   |     20 |     4 |
 * | 4000003  | Site21   |     21 |     4 |
 * +----------+----------+--------+-------+
*/