#ifndef EPOLL_HPP
# define EPOLL_HPP

# include "Config.hpp"
# include <set>
# include <sys/epoll.h>
# include <vector>

class Epoll {
	public:
		Epoll();
		Epoll(const Config& config);
		~Epoll();
		void AddListener(int port);
		void Add(int fd,uint32_t events);
		void Mod(int fd,uint32_t events);
		void Del(int fd);
		void Accept(int fd, int& client_fd, int& port);
		const std::vector<epoll_event>& WaitEvents(int timeout = -1);
		bool IsListen(int fd);

	private:
		int 											epfd_;
		static const int 					max_events_ = 64;
		std::set<int> 						listen_fds_;
		std::map<int, int> 				fd_to_port_;
		std::vector<epoll_event> 	events_;
};

#endif
