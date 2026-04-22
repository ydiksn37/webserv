#ifndef EPOLL_HPP
#define EPOLL_HPP

#include <cstdint>
#include <set>
#include <sys/epoll.h>
class Epoll
{
	public:
		Epoll();
		~Epoll();
		void AddListener(int port);
		void Add(int fd,uint32_t events);
		void Mod(int fd,uint32_t events);
		void Del(int fd);
	private:
		int epfd_;
		static const int max_events_ = 64;
		struct epoll_event events_[max_events_];
		std::set<int> listen_fds;
};

#endif
