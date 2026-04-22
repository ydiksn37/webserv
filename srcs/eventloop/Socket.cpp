#include <cerrno>
#include <csignal>
#include <cstring>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>

int CreateSocket(int port)
{
	int sd_listen = socket(AF_INET, SOCK_STREAM, 0);
	if(sd_listen < 0)
		throw std::runtime_error(strerror(errno));
	struct sockaddr_in addr_server;
	addr_server.sin_family = AF_INET;
	addr_server.sin_port = htons(port);
	addr_server.sin_addr.s_addr = INADDR_ANY;
	int yes = 1;
	if(setsockopt(sd_listen, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))<0)
		throw std::runtime_error(strerror(errno));
	signal(SIGPIPE, SIG_IGN);
	if(bind(sd_listen,(sockaddr *)&addr_server, sizeof(addr_server)) < 0)
		throw std::runtime_error(strerror(errno));
	if(listen(sd_listen, SOMAXCONN) < 0)
		throw std::runtime_error(strerror(errno));

	return sd_listen;
}
