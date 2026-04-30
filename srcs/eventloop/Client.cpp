#include "Client.hpp"
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <string>
#include <unistd.h>
#include "engine.hpp"

bool endswith(std::string str, std::string suffix)
{
	if(str.size() < suffix.size())
		return false;
	return str.substr(str.size()-suffix.size()) == suffix;
}

Client::Client(const Config& config):config_(config){}

int Client::Read(int fd)
{
	char tmp_buffer[buffer_size];
	int read_size = read(fd, tmp_buffer, buffer_size);
	if(read_size <= 0)
	{
		client_.erase(fd);
		return -1;
	}
	client_[fd].request.parse(std::string(tmp_buffer,read_size));

	while(client_[fd].request.isCompleted())
	{
		HttpResponse response = engine(config_, client_[fd].request);
		client_[fd].write_buffer.append(response.serialize());
		client_[fd].request.clear();
		client_[fd].request.parse("");
	}
	return 0;
}

void Client::Write(int fd)
{
	int write_size = write(fd, client_[fd].write_buffer.c_str(), client_[fd].write_buffer.size());
	std::cout<<"\033[32m[WRITE] \033[0m"<<std::endl;
	std::cout<<client_[fd].write_buffer.substr(0,write_size)<<std::endl;
	client_[fd].write_buffer.erase(0,write_size);
}

bool Client::WriteBegin(int fd)
{
	if(client_[fd].current_epoll_events==EPOLLIN && client_[fd].write_buffer.size())
	{
		client_[fd].current_epoll_events=EPOLLIN | EPOLLOUT;
		return true;
	}
	return false;
}

bool Client::WriteEnd(int fd)
{
	if(client_[fd].current_epoll_events==(EPOLLIN | EPOLLOUT) && client_[fd].write_buffer.empty())
	{
		client_[fd].current_epoll_events=EPOLLIN;
		return true;
	}
	return false;
}
