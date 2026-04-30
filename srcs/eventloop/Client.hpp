#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <map>
#include <string>
#include <sys/epoll.h>
#include "Config.hpp"


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
			std::string read_buffer;
			bool header_parsed;
			unsigned header_length;
			unsigned body_length;
			bool chunked;
			std::string header; // Engineができたらクラスに置き換える
			std::string unchunked_body;
			std::string write_buffer;
			unsigned current_epoll_events;
			ClientData():header_parsed(false),header_length(0),body_length(0),chunked(false),current_epoll_events(EPOLLIN){}
		};
		std::map<int,ClientData> client_;
};

#endif
