#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <map>
#include <string>
#include <sys/epoll.h>


class Client
{
	public:
		Client();
		int Read(int fd);
		void Write(int fd);
		bool WriteBegin(int fd);
		bool WriteEnd(int fd);
	private:
		static const unsigned buffer_size = 8096;
		struct ClientData
		{
			std::string read_buffer;
			bool header_parsed;
			unsigned header_length;
			unsigned body_length;
			std::string write_buffer;
			unsigned current_epoll_events;
			ClientData():header_parsed(false),header_length(0),body_length(0),current_epoll_events(EPOLLIN){}
		};
		std::map<int,ClientData> client_;
};

#endif
