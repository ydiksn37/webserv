#include "Server.hpp"
#include <asm-generic/socket.h>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

Server::Server(int port)
{
	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(_socket_fd < 0)
		throw std::runtime_error(strerror(errno));
	_addr_server.sin_family = AF_INET;
	_addr_server.sin_port = htons(port);
	_addr_server.sin_addr.s_addr = INADDR_ANY;
	int yes = 1;
	if(setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))<0)
		throw std::runtime_error(strerror(errno));
	signal(SIGPIPE, SIG_IGN);
	if(bind(_socket_fd,(sockaddr *)&_addr_server, sizeof(_addr_server)) < 0)
		throw std::runtime_error(strerror(errno));
	_addr_len = sizeof(struct sockaddr_in);
}

Server::~Server()
{
	close(_socket_fd);
	close(_client_fd);
}

void Server::Connect()
{
	if(listen(_socket_fd, SOMAXCONN) < 0)
		throw std::runtime_error(strerror(errno));
	_client_fd = accept(_socket_fd, (sockaddr *)&_addr_client, &_addr_len);
	if(_client_fd < 0)
		throw std::runtime_error(strerror(errno));
	if(fcntl(_client_fd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error(strerror(errno));
	// setsockopt(_client_fd, SOL_SOCKET, SO_RCVTIMEO, &_timeout, sizeof(_timeout));
	// timeoutはepollで行う
}

void Server::Run()
{
	char buf[_buffer_size];
	int rd;

	std::string response = 
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: 47\r\n"
		"Connection: keep-alive\r\n"
		"\r\n"
		"<html><body><h1>Hello World!</h1></body></html>";
	while (1) {
		rd = read(_client_fd, buf, _buffer_size - 1);
		if(rd > 0)
		{
			buf[rd] = '\0';
			std::cout<<buf;
			write(_client_fd, response.c_str(), response.size());
		}
		else if(rd == 0)
		{
			std::cout<<"Client disconnected."<<std::endl;
			break;
		}
		usleep(100000);
	}
}
