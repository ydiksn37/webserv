#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <exception>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <ostream>
#include <string>
#include <sys/select.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define N_FDS (5)

bool endswith(std::string str, std::string suffix)
{
	if(str.size() < suffix.size())
		return false;
	return str.substr(str.size()-suffix.size()) == suffix;
}

std::string Engine(const std::string& request)
{
	(void) request;

	std::string response = 
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: 47\r\n"
		"Connection: keep-alive\r\n"
		"\r\n"
		"<html><body><h1>Hello World!</h1></body></html>";
	return response;
}

int CreateSocket(int port)
{
	int sd_listen = socket(AF_INET, SOCK_STREAM, 0);
	if(sd_listen < 0)
		throw std::runtime_error(strerror(errno));
	struct sockaddr_in addr_server;
	addr_server.sin_family = AF_INET;
	addr_server.sin_port = htons(port);
	addr_server.sin_addr.s_addr = INADDR_ANY;
	int yes = 1;
	if(setsockopt(sd_listen, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))<0)
		throw std::runtime_error(strerror(errno));
	signal(SIGPIPE, SIG_IGN);
	if(bind(sd_listen,(sockaddr *)&addr_server, sizeof(addr_server)) < 0)
		throw std::runtime_error(strerror(errno));
	if(listen(sd_listen, SOMAXCONN) < 0)
		throw std::runtime_error(strerror(errno));

	return sd_listen;
}

struct Client
{
	std::string read_buffer;
	std::string write_buffer;
};

void EventLoop()
{
	struct epoll_event events[N_FDS];
	int epfd;
	struct epoll_event ev;
	int sd_listen = CreateSocket(8080);
	char buf[8192];
	std::map<int, struct Client> client;

	epfd = epoll_create(N_FDS);
	if(epfd < 0)
		throw strerror(errno);
	ev.events = EPOLLIN;
	ev.data.fd = sd_listen;
	epoll_ctl(epfd, EPOLL_CTL_ADD, sd_listen, &ev);
	while(1)
	{
		int n_events = epoll_wait(epfd, events, N_FDS, -1);
		if(n_events <= 0)
			throw strerror(errno);

		for(int i=0;i<n_events;i++)
		{
			if(events[i].data.fd < 0)
				continue;
			else if(events[i].data.fd == sd_listen)
			{
				int sd_client = accept(sd_listen, NULL, NULL);
				ev.events = EPOLLIN;
				ev.data.fd = sd_client;

				epoll_ctl(epfd, EPOLL_CTL_ADD, sd_client, &ev);
			}
			else
			{
				int sd_client = events[i].data.fd;
				if(events[i].events & EPOLLIN)
				{
					int read_size = recv(sd_client, buf, sizeof(buf)-1,0);
					if(read_size <= 0)
						close(sd_client);
					else if(read_size > 0)
					{
						buf[read_size] = '\0';
						client[sd_client].read_buffer.append(buf);
						if(endswith(client[sd_client].read_buffer,"\r\n\r\n"))
						{
							client[sd_client].write_buffer = Engine(client[sd_client].read_buffer);
							ev.events = EPOLLOUT;
							ev.data.fd = sd_client;
							epoll_ctl(epfd, EPOLL_CTL_MOD, sd_client, &ev);
						}
					}
				}
				else if(events[i].events & EPOLLOUT)
				{
					int write_size = send(sd_client, client[sd_client].write_buffer.c_str(), client[sd_client].write_buffer.size(), 0);
					client[sd_client].write_buffer.erase(0,write_size);
					if(client[sd_client].write_buffer.empty())
					{
						client.erase(sd_client);
						close(sd_client);
					}
				}
			}
		}
	}
}

int main()
{
	try {
		EventLoop();
	} catch (const std::exception& e) {
		std::cout<<e.what()<<std::endl;
		return 1;
	}
}
