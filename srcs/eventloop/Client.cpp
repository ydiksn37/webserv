#include "Client.hpp"
#include <cstddef>
#include <cstdlib>
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

void  Client::GenResponse(int fd,std::string request)
{
	std::cout<<"\033[95m[READ]\033[0m "<<std::endl;
	std::cout<<request<<std::endl;
	client_[fd].write_buffer.append(Engine(request));
	client_[fd].header_parsed = false;
	client_[fd].header_length = 0;
	client_[fd].body_length = 0;
	client_[fd].chunked = false;
}


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
			client_[fd].header = client_[fd].read_buffer.substr(0,client_[fd].header_length);
			client_[fd].read_buffer.erase(0,client_[fd].header_length);
			if(IsChunked(client_[fd].header))
				client_[fd].chunked = true;
			else
				client_[fd].body_length = GetContentLength(client_[fd].header);
		}
		if(client_[fd].header_parsed)
		{
			if(client_[fd].chunked)
			{
				size_t pos = client_[fd].read_buffer.find("\r\n");
				if(pos == std::string::npos)
					break;
				char *endptr = NULL;
				size_t size = std::strtoul(client_[fd].read_buffer.c_str(),&endptr,16);
				if(endptr == client_[fd].read_buffer.c_str() || (*endptr !='\r' && *endptr != '\0'))
				{
					client_.erase(fd);
					return -1;
				}
				if(size==0)
				{
					client_[fd].read_buffer.erase(0,pos+4);
					GenResponse(fd, client_[fd].header+client_[fd].unchunked_body);
				}
				else if(pos+2+size <= client_[fd].read_buffer.size())
				{
					client_[fd].unchunked_body.append(client_[fd].read_buffer.substr(pos+2,size));
					client_[fd].read_buffer.erase(0,pos+2+size+2);
				}
				else
					break;
			}
			else
			{
				if(client_[fd].read_buffer.size() >= client_[fd].body_length)
				{
					GenResponse(fd,client_[fd].header + client_[fd].read_buffer.substr(0,client_[fd].body_length));
					client_[fd].read_buffer.erase(0,client_[fd].body_length);
				}
				else
					break;
			}
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
