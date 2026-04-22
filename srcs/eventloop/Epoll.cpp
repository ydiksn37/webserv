#include "Epoll.hpp"
#include "eventloop_int.hpp"
#include <cerrno>
#include <cstring>
#include <sys/epoll.h>
#include <unistd.h>

Epoll::Epoll()
{
	epfd_ = epoll_create(1);
	if(epfd_ < 0)
		throw strerror(errno);
}

Epoll::~Epoll()
{
}

void Epoll::AddListener(int port)
{
	int listen_fd = CreateSocket(8080);
	Add(listen_fd, EPOLLIN);
	listen_fds.insert(listen_fd);
}

void Epoll::Add(int fd,uint32_t events)
{
	struct epoll_event ev;
	
	ev.events = EPOLLIN;
	ev.data.fd = fd;

	epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev);
}

void Epoll::Mod(int fd,uint32_t events)
{
	struct epoll_event ev;
	
	ev.events = EPOLLIN;
	ev.data.fd = fd;

	epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev);
}

void Epoll::Del(int fd)
{
	close(fd);
}
