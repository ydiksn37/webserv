#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <map>
#include <string>
#include <sys/epoll.h>
#include "Config.hpp"
#include "../Http/HttpRequest.hpp"

class Client
{
	public:
		Client(const Config& config);
		int Read(int fd);
		void Write(int fd);
		bool WriteBegin(int fd);
		bool WriteEnd(int fd);
	private:
		static const unsigned buffer_size = 8096;
		void GenResponse(int fd, std::string request);
		Config config_;
		struct ClientData
		{
			HttpRequest request;
			std::string write_buffer;
			unsigned current_epoll_events;
			ClientData():current_epoll_events(EPOLLIN){}
		};
		std::map<int,ClientData> client_;
};

#endif
