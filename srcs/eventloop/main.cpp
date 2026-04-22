
#include "Client.hpp"
#include "Epoll.hpp"
#include <iostream>
#include <unistd.h>

void EventLoop(Epoll& ep)
{
	Client client;

	while(1)
	{
		const std::vector<epoll_event>& events = ep.WaitEvents();

		for(unsigned i=0;i<events.size();i++)
		{
			if(events[i].data.fd < 0) //TODO バグなので対処すべきかも
				continue;

			if(ep.IsListen(events[i].data.fd))
				ep.Accept(events[i].data.fd);
			else
			{
				int client_fd = events[i].data.fd;
				if(events[i].events & EPOLLIN)
				{
					if(client.Read(client_fd) <= 0)
						close(client_fd);
					else if(!client.IsRead(client_fd)) // Readが終わったら
						ep.Mod(client_fd, EPOLLOUT);
				}
				else if(events[i].events & EPOLLOUT)
				{
					client.Write(client_fd);
					if(!client.IsRead(client_fd)) // writeが終わったら
						close(client_fd);
				}
			}
		}
	}
}

int main()
{
	Epoll ep;
	ep.AddListener(8080);
	try {
		EventLoop(ep);
	} catch (const std::exception& e) {
		std::cout<<e.what()<<std::endl;
		return 1;
	}
}
