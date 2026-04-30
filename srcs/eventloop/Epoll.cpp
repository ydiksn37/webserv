#include "Epoll.hpp"
#include "Socket.hpp"
#include "Config.hpp"
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>
#include "EventLoop.hpp"

Epoll::Epoll()
{
	epfd_ = epoll_create(1);
	if(epfd_ < 0)
		throw EventLoopException(strerror(errno));
	events_.resize(max_events_);
}

Epoll::Epoll(const Config& config)
{
	epfd_ = epoll_create(1);
	if(epfd_ < 0)
		throw EventLoopException(strerror(errno));
	events_.resize(max_events_);

	std::vector<ServerContext> servers = config.getServers();
	std::vector<int> ports;

	for(unsigned i=0;i<servers.size();i++)
		ports.push_back(servers[i].getPort());
	std::sort(ports.begin(),ports.end());
	ports.erase(std::unique(ports.begin(),ports.end()),ports.end());
	for(unsigned i=0;i<ports.size();i++)
		AddListener(ports[i]);
}

Epoll::~Epoll(){}

void Epoll::AddListener(int port)
{
	int listen_fd = CreateSocket(port);
	Add(listen_fd, EPOLLIN);
	listen_fds_.insert(listen_fd);
}

void Epoll::Add(int fd,uint32_t events)
{
	struct epoll_event ev;
	
	ev.events = events;
	ev.data.fd = fd;

	epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev);
}

void Epoll::Mod(int fd,uint32_t events)
{
	struct epoll_event ev;
	
	ev.events = events;
	ev.data.fd = fd;

	epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev);
}

void Epoll::Del(int fd)
{
	close(fd);
}

void Epoll::Accept(int fd)
{
	int client_fd = accept(fd, NULL, NULL);
	if(client_fd < 0)
		throw EventLoopException(strerror(errno));
	Add(client_fd, EPOLLIN);
}

const std::vector<epoll_event>& Epoll::WaitEvents()
{
	events_.resize(max_events_);
	int events_size = epoll_wait(epfd_,events_.data() , events_.size(), -1);
	if(events_size < 0)
		throw EventLoopException(strerror(errno));
	events_.resize(events_size);
	return events_;
}

bool Epoll::IsListen(int fd)
{
	return listen_fds_.count(fd);
}
