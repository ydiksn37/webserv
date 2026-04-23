#include "Client.hpp"
#include "Epoll.hpp"
#include <iostream>
#include <ostream>
#include <sys/epoll.h>
#include <unistd.h>
#include "eventloop_int.hpp"

void EventLoop(Epoll& ep)
{
	Client client;

	while(1)
	{
		const std::vector<epoll_event>& events = ep.WaitEvents();

		for(unsigned i=0;i<events.size();i++)
		{
			std::cout<<ep.IsListen(events[i].data.fd)<<":"<<events[i].data.fd<<": "<<get_epoll_events_str(events[i].events)<<std::endl;
			if(ep.IsListen(events[i].data.fd))
				ep.Accept(events[i].data.fd);
			else
			{
				int client_fd = events[i].data.fd;
				if(events[i].events & EPOLLIN)
				{
					if(client.Read(client_fd) <= 0)
						close(client_fd);
					else if(!client.IsRead(client_fd))
						ep.Mod(client_fd, EPOLLOUT);
				}
				else if(events[i].events & EPOLLOUT)
				{
					client.Write(client_fd);
					if(client.IsRead(client_fd))
					{
						ep.Mod(client_fd, EPOLLIN);
					}
				}
			}
		}
	}
}

int main()
{
	try {
		Epoll ep;
		ep.AddListener(8080);
		ep.AddListener(8081);
		EventLoop(ep);
	} catch (const std::exception& e) {
		std::cout<<e.what()<<std::endl;
		return 1;
	}
}
