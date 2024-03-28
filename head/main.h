#pragma once

#include "common.h"

void sigint_handler(int signo);
void sigchld_handler(int signo);
void sigchld_handler_old(int signo);
void sigsegfault_handler(int signal, siginfo_t *si, void *arg);
void setChldSignal();
void setIntSignal();
void setSegFaultSignal();
void clearMessageQueue();
void createDataFile(std::string filename);
core::common::Mapper add_mapper(int pid, u_short addr);
void print_mapper(core::common::Mapper* mapper);
void search_mapper(core::common::Mapper* mapper, pid_t &pid, int idx, u_short addr);
void search_mapper(core::common::Mapper* mapper, std::vector<pid_t> &pids, int idx, u_short addr);
void search_mapper(core::common::Mapper* mapper, u_short &addr, pid_t pid, int idx);
bool delete_mapper(core::common::Mapper* mapper, int idx, int pid);
void write_mapper(std::string filename, core::common::Mapper* mapper);
void read_mapper(std::string filename, core::common::Mapper* mapper);
int getTotalLine(std::string name);
void init_rtu_status();
void set_rtu_status(u_short sid, DATA status);

///////////////////////////////////////////////////////////////////////////////////
std::priority_queue<u_short, std::vector<u_short>, std::greater<u_short>> cmdAddrQueue;
core::common::Mapper rtu_mapper_list[MAX_POOL];
core::common::Mapper cmd_addr_list[MAX_POOL];

///////////////////////////////////////////////////////////////////////////////////
std::pair<pid_t, u_short> add_pair(pid_t pid, u_short addr);
void read_pair(std::string filename, std::set<std::pair<pid_t, u_short>> &s);
///////////////////////////////////////////////////////////////////////////////////
