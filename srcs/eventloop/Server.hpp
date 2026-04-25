#ifndef SERVER_HPP
#define SERVER_HPP
#include <netinet/in.h>
#include <sys/socket.h>

class Server
{
	public:
		Server(int port);
		~Server();
		void Connect();
		void Run();
	private:
		int _socket_fd;
		int _client_fd;
		struct sockaddr_in _addr_server;
		struct sockaddr_in _addr_client;
		socklen_t _addr_len;
		static const unsigned _buffer_size=8096;
};
#endif
