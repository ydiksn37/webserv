#ifndef EVENT_LOOP
#define EVENT_LOOP
#include <string>

int CreateSocket(int port);
std::string Engine(const std::string& request); // for test
std::string get_epoll_events_str(uint32_t events);

#endif
