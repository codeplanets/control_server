#include <unistd.h>

#include "client.h"
#include "packetizer.h"
#include "socketexception.h"

namespace core {
    Client::Client(ServerSocket& sock)
    : newSock(sock)
    , m_isCreatedMq(false) {
        
    }
    
    Client::~Client() {
        if (m_isCreatedMq) {
            mq.close();
        }
        updateStatus(false);
    }

    /**
     * @return true if SiteCode is available, false otherwise
    */
    bool Client::isSiteCodeAvailable() {
        string siteCode = find_rtu_addr(this->scode);
        if ( siteCode == NOT_FOUND) {
            return false;
        }
        syslog(LOG_DEBUG, "Site Code is available. : %s", siteCode.c_str());
        return true;
    }

    bool Client::createMessageQueue(std::string mq_name) {
        bool isCreated = mq.open(mq_name, getpid());
        m_isCreatedMq = isCreated;
        return isCreated;
    }
    
    std::string Client::find_rtu_addr(SiteCode scode) {
        string addr = NOT_FOUND;
        Database db;
        ECODE ecode = db.db_init("localhost", 3306, "rcontrol", "rcontrol2024", "RControl");
        if (ecode!= EC_SUCCESS) {
            syslog(LOG_ERR, "DB Connection Error!");
            exit(EXIT_FAILURE);
        }
        char* siteCode = scode.getSiteCode();
        string query = "select * from RSite";
        query += " where SiteCode = '";
        query += siteCode;
        query += "';";
        syslog(LOG_DEBUG, "Query : %s", query.c_str());
        delete[] siteCode;

        // Query Data
        MYSQL_ROW sqlrow;
        MYSQL_RES* pRes;
        ecode = db.db_query(query.c_str(), &pRes);
        if (ecode != EC_SUCCESS) {
            syslog(LOG_ERR, "DB Query Error!");
            exit(EXIT_FAILURE);
        }
        syslog(LOG_DEBUG, "+----------+----------+--------+-------+");
        syslog(LOG_DEBUG, "| SiteCode | SiteName | SiteID | Basin |");
        syslog(LOG_DEBUG, "+----------+----------+--------+-------+");
        try {
            while ((sqlrow = db.db_fetch_row(pRes)) != NULL) {
                syslog(LOG_DEBUG, "|%9s |%9s |%7s |%6s |", sqlrow[0], sqlrow[1], sqlrow[2], sqlrow[3]);
                addr = sqlrow[2];
            }
        } catch (exception& e) {
            syslog(LOG_ERR, "DB Fetch Error!");
            cout << e.what() << endl;
        }
        syslog(LOG_DEBUG, "+----------+----------+--------+-------+");
        return addr;
    }

    core::common::MAPPER Client::add_mapper(int pid, u_short addr) {
        core::common::MAPPER mapper;
        mapper.pid = pid;
        mapper.addr = addr;
        return mapper;
    }

    void Client::print_mapper(core::common::MAPPER* mapper) {
        for (int i = 0; i < MAX_POOL; i++) {
            if (mapper[i].pid != 0) {
                syslog(LOG_DEBUG, "Mapper : %d 0x%02X", mapper[i].pid, mapper[i].addr);
            }
        }
    }

    void Client::search_mapper(core::common::MAPPER* mapper, pid_t &pid, int idx, u_short addr) {
        core::common::MAPPER* map;
        for (map = mapper; map < mapper + idx; map++) {
            if (map->addr == addr) {
                syslog(LOG_DEBUG, "Found Mapper : %d 0x%02X", map->pid, map->addr);

                pid = map->pid;
            }
        }
    }

    void Client::search_mapper(core::common::MAPPER* mapper, std::vector<pid_t> &pids, int idx, u_short addr) {
        core::common::MAPPER* map;
        for (map = mapper; map < mapper + idx; map++) {
            if (map->addr == addr) {
                syslog(LOG_DEBUG, "Found Mapper : %d 0x%02X", map->pid, map->addr);

                pids.push_back(map->pid);
            }
        }
    }

    bool Client::delete_mapper(core::common::MAPPER* mapper, int idx, int pid) {
        bool ret = false;
        core::common::MAPPER* map;
        for (map = mapper; map < mapper + idx; map++) {
            if (map->pid == pid) {
                map->pid = 0;
                ret = true;
            }
        }
        return ret;
    }
    void Client::write_mapper(std::string filename, core::common::MAPPER* mapper) {
        FILE * f = fopen(filename.c_str(), "a");
        for (int i = 0; i < MAX_POOL; i++) {
            if (mapper[i].pid != 0) {
                fprintf(f, "%d %hd\n", mapper[i].pid, mapper[i].addr);
            }
        }
        fclose(f);
    }

    void Client::read_mapper(std::string filename, core::common::MAPPER* mapper) {
        FILE * f = fopen(filename.c_str(), "r");
        for (int i = 0; i < MAX_POOL; i++) {
            fscanf(f, "%d %hd", &mapper[i].pid, &mapper[i].addr);
        }
        fclose(f);
    }

    int Client::getTotalLine(string name) {
        FILE *fp;
        int line=0;
        char c;
        fp = fopen(name.c_str(), "r");
        while ((c = fgetc(fp)) != EOF) {
            if (c == '\n') {
                line++;
            }
        }
        fclose(fp);
        return(line);
    }

    bool Client::updateStatus(bool status) { return true; }
    bool Client::updateDatabase(bool status) { return true; }
}
