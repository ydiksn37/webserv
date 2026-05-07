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
#include <fcntl.h>
#include <stdexcept>

Epoll::Epoll() {
	epfd_ = epoll_create(1);
	if(epfd_ < 0)
		throw EventLoopException(strerror(errno));
	events_.resize(max_events_);
}

Epoll::Epoll(const Config& config) {
	epfd_ = epoll_create(1);
	if(epfd_ < 0)
		throw EventLoopException(strerror(errno));
	events_.resize(max_events_);

	std::vector<ServerContext> servers = config.getServers();
	std::vector<int> ports;

	for(unsigned i = 0; i < servers.size(); i++)
		ports.push_back(servers[i].getPort());
	std::sort(ports.begin(), ports.end());
	ports.erase(std::unique(ports.begin(), ports.end()), ports.end());
	for(unsigned i = 0; i < ports.size(); i++)
		AddListener(ports[i]);
}

Epoll::~Epoll() {}

void Epoll::AddListener(int port) {
	int listen_fd = CreateSocket(port);
	Add(listen_fd, EPOLLIN);
	listen_fds_.insert(listen_fd);
	fd_to_port_[listen_fd] = port;
}

void Epoll::Add(int fd,uint32_t events) {
	struct epoll_event ev;

	ev.events = events;
	ev.data.fd = fd;

	epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev);
}

void Epoll::Mod(int fd,uint32_t events) {
	struct epoll_event ev;

	ev.events = events;
	ev.data.fd = fd;

	epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev);
}

void Epoll::Del(int fd) {
	epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL);
	close(fd);
}

void Epoll::Accept(int fd, int& client_fd, int& port) {
	client_fd = accept(fd, NULL, NULL);
	if(client_fd < 0)
		throw EventLoopException(strerror(errno));
	fcntl(client_fd, F_SETFL, O_NONBLOCK);
	Add(client_fd, EPOLLIN);
	port = fd_to_port_[fd];
}

const std::vector<epoll_event>& Epoll::WaitEvents(int timeout) {
	events_.resize(max_events_);
	int events_size = epoll_wait(epfd_, &events_[0], events_.size(), timeout);
	if(events_size < 0) {
		if (errno == EINTR) {
			events_.clear();
			return events_;
		}
		throw EventLoopException(strerror(errno));
	}
	events_.resize(events_size);
	return events_;
}

bool Epoll::IsListen(int fd) {
	return listen_fds_.count(fd);
}
