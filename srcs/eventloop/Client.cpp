#include "Client.hpp"
#include <cstddef>
#include <iostream>
#include <ostream>
#include <string>
#include <unistd.h>
#include "eventloop_int.hpp"

bool endswith(std::string str, std::string suffix)
{
	if(str.size() < suffix.size())
		return false;
	return str.substr(str.size()-suffix.size()) == suffix;
}

Client::Client(){}

int Client::Read(int fd)
{
	char tmp_buffer[buffer_size];
	int read_size = read(fd, tmp_buffer, buffer_size);
	if(read_size <= 0)
	{
		client_.erase(fd);
		return -1;
	}
	client_[fd].read_buffer.append(tmp_buffer,read_size);

	while(true)
	{
		if(!client_[fd].header_parsed)
		{
			size_t pos = client_[fd].read_buffer.find("\r\n\r\n");
			if(pos == std::string::npos)
				break;
			client_[fd].header_parsed = true;
			client_[fd].header_length = pos + 4;
			client_[fd].body_length = GetContentLength(client_[fd].read_buffer.substr(0,client_[fd].header_length));
		}
		if(client_[fd].header_parsed)
		{
			unsigned total_length = client_[fd].header_length + client_[fd].body_length;
			if(client_[fd].read_buffer.size() >= total_length)
			{
				std::string request = client_[fd].read_buffer.substr(0,total_length);
				std::cout<<"\033[95m[READ]\033[0m "<<std::endl;
				std::cout<<request<<std::endl;
				client_[fd].write_buffer.append(Engine(request));
				client_[fd].read_buffer.erase(0,total_length);
				client_[fd].header_parsed = false;
				client_[fd].header_length = 0;
				client_[fd].body_length = 0;

			}
			else
				break;
		}
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
