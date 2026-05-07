# include <cerrno>
# include <csignal>
# include <cstring>
# include <netinet/in.h>
# include <stdexcept>
# include <sys/socket.h>
# include <fcntl.h>
#include "EventLoop.hpp"

int CreateSocket(int port) {
	int sd_listen = socket(AF_INET, SOCK_STREAM, 0);
	if(sd_listen < 0)
		throw EventLoopException(strerror(errno));
	struct sockaddr_in addr_server;
	addr_server.sin_family = AF_INET;
	addr_server.sin_port = htons(port);
	addr_server.sin_addr.s_addr = INADDR_ANY;
	int yes = 1;
	if(setsockopt(sd_listen, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
		throw EventLoopException(strerror(errno));
	signal(SIGPIPE, SIG_IGN);
	if(bind(sd_listen, (sockaddr *)&addr_server, sizeof(addr_server)) < 0)
		throw EventLoopException(strerror(errno));
	if (listen(sd_listen, SOMAXCONN) < 0)
		throw EventLoopException(strerror(errno));

	fcntl(sd_listen, F_SETFL, O_NONBLOCK);

	return sd_listen;
}
