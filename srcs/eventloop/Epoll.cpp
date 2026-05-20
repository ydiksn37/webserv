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
	std::set<std::pair<std::string, int> > listen_pairs;

	for(unsigned i = 0; i < servers.size(); i++)
		listen_pairs.insert(std::make_pair(servers[i].getHost(), servers[i].getPort()));

	for(std::set<std::pair<std::string, int> >::iterator it = listen_pairs.begin(); it != listen_pairs.end(); ++it)
		AddListener(it->second, it->first);
}

Epoll::~Epoll() {}

void Epoll::AddListener(int port, const std::string& host) {
	int listen_fd = CreateSocket(port, host);
	Add(listen_fd, EPOLLIN);
	listen_fds_.insert(listen_fd);
	fd_to_endpoint_[listen_fd] = ListenEndpoint(host, port);
}

void Epoll::Add(int fd,uint32_t events) {
	struct epoll_event ev;

	ev.events = events;
	ev.data.fd = fd;

	if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) < 0)
		throw EventLoopException(strerror(errno));
}

void Epoll::Mod(int fd,uint32_t events) {
	struct epoll_event ev;

	ev.events = events;
	ev.data.fd = fd;

	if (epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev) < 0)
		throw EventLoopException(strerror(errno));
}

void Epoll::Del(int fd) {
	if (epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL) < 0) {
		close(fd);
		throw EventLoopException(strerror(errno));
	}
	close(fd);
}

void Epoll::Accept(int fd, int& client_fd, ListenEndpoint& endpoint) {
	client_fd = accept(fd, NULL, NULL);
	if(client_fd < 0)
		throw EventLoopException(strerror(errno));
	fcntl(client_fd, F_SETFL, O_NONBLOCK);
	Add(client_fd, EPOLLIN);
	endpoint = fd_to_endpoint_[fd];
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
