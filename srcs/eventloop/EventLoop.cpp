#include "Client.hpp"
#include "Epoll.hpp"
#include "eventloop_int.hpp"
#include "Config.hpp"
#include <iostream>
#include <ostream>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

void EventLoop(const Config& config) {
	Client client(config);
	Epoll ep(config);

	while(1) {
		const std::vector<epoll_event>& events = ep.WaitEvents(1000);

		// タイムアウトしたCGIのクリーンアップ
		std::vector<int> timed_out_fds = client.HandleCgiTimeout();
		for (size_t i = 0; i < timed_out_fds.size(); ++i) {
			ep.Del(timed_out_fds[i]);
		}

		for(unsigned i = 0; i < events.size(); i++) {
			int fd = events[i].data.fd;
			if(ep.IsListen(fd)) {
				int client_fd, port;
				ep.Accept(fd, client_fd, port);
				client.SetLocalPort(client_fd, port);
			}
			else {
				int client_fd = client.GetClientFdFromPipe(fd);
				if (client_fd != -1) {
					// CGIパイプのI/O処理
					bool closed = false;
					if (events[i].events & (EPOLLIN | EPOLLHUP | EPOLLERR)) {
						bool closed_event = events[i].events & (EPOLLHUP | EPOLLERR);
						if (client.ReadCgi(fd, closed_event) == 1) {
							ep.Del(fd);
							closed = true;
						}
					}
					if (!closed && (events[i].events & EPOLLOUT)) {
						if (client.WriteCgi(fd) == 1) {
							ep.Del(fd);
							closed = true;
						}
					}
					// CGIの結果が書き込みバッファに入った可能性があるので、client_fd の監視状態を更新
					if (client.WriteBegin(client_fd))
						ep.Mod(client_fd, EPOLLIN | EPOLLOUT);
				}
				else {
					// クライアントソケットのI/O処理
					if(events[i].events & EPOLLIN) {
						std::vector<Client::PipeInfo> new_pipes;
						if(client.Read(fd, new_pipes) < 0)
							ep.Del(fd);
						else {
							for (size_t j = 0; j < new_pipes.size(); ++j)
								ep.Add(new_pipes[j].fd, new_pipes[j].events);
							if(client.WriteBegin(fd))
								ep.Mod(fd, EPOLLIN | EPOLLOUT);
						}
					}
					if(events[i].events & EPOLLOUT) {
						client.Write(fd);
						if(client.WriteEnd(fd)) {
							if (client.ShouldClose(fd))
								ep.Del(fd);
							else
								ep.Mod(fd, EPOLLIN);
						}
					}
				}
			}
		}
	}
}
