#ifndef EPOLL_HPP
#define EPOLL_HPP
#include <set>
#include <sys/epoll.h>
#include <vector>

class Epoll
{
	public:
		Epoll();
		~Epoll();
		void AddListener(int port);
		void Add(int fd,uint32_t events);
		void Mod(int fd,uint32_t events);
		void Del(int fd);
		void Accept(int fd);
		const std::vector<epoll_event>& WaitEvents(); //TODO ブロックしてるのでタイムアウトを導入したほうがいいかも
		bool IsListen(int fd);
	private:
		int epfd_;
		static const int max_events_ = 64;
		std::set<int> listen_fds_;
		std::vector<epoll_event> events_;
};

#endif
