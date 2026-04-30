#ifndef EVENT_LOOP
#define EVENT_LOOP
#include <string>

int CreateSocket(int port);
std::string get_epoll_events_str(unsigned events);

#endif
