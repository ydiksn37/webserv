#include "Client.hpp"
#include "Epoll.hpp"
#include <iostream>
#include <ostream>
#include <sys/epoll.h>
#include <unistd.h>
#include "Config.hpp"

std::string get_epoll_events_str(uint32_t events) {
	std::string str = "";

	if (events & EPOLLIN)      str += "EPOLLIN ";
	if (events & EPOLLOUT)     str += "EPOLLOUT ";

#ifdef EPOLLRDHUP
	if (events & EPOLLRDHUP)   str += "EPOLLRDHUP ";
#endif

	if (events & EPOLLERR)     str += "EPOLLERR ";
	if (events & EPOLLHUP)     str += "EPOLLHUP ";
	if (events & EPOLLPRI)     str += "EPOLLPRI ";

	if (events & EPOLLET)      str += "EPOLLET ";
	if (events & EPOLLONESHOT) str += "EPOLLONESHOT ";

	if (str.empty())           str = "UNKNOWN ";

	return str;
}

void EventLoop(const Config& config)
{
	Client client(config);
	Epoll ep(config);

	while(1)
	{
		const std::vector<epoll_event>& events = ep.WaitEvents();

		for(unsigned i=0;i<events.size();i++)
		{
			std::cout<<"\033[36m"<<get_epoll_events_str(events[i].events)<<": \033[0m"<<(ep.IsListen(events[i].data.fd)?"Listen_fd":"Client_fd")<<":"<<events[i].data.fd<<std::endl;
			if(ep.IsListen(events[i].data.fd))
				ep.Accept(events[i].data.fd);
			else
			{
				int client_fd = events[i].data.fd;
				if(events[i].events & EPOLLIN)
				{
					if(client.Read(client_fd) < 0)
					{
						std::cout<<"Client_fd: "<<client_fd<<" Disconnected."<<std::endl;
						close(client_fd);
					}
					else if(client.WriteBegin(client_fd))
						ep.Mod(client_fd, EPOLLIN | EPOLLOUT);
				}
				if(events[i].events & EPOLLOUT)
				{
					client.Write(client_fd);
					if(client.WriteEnd(client_fd))
						ep.Mod(client_fd, EPOLLIN);
				}
			}
		}
	}
}
