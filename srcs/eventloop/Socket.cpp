# include <cerrno>
# include <csignal>
# include <cstring>
# include <netinet/in.h>
# include <sys/socket.h>
# include <fcntl.h>
# include <netdb.h>
# include <sstream>
#include "EventLoop.hpp"

int CreateSocket(int port, const std::string& host) {
	struct addrinfo hints;
	struct addrinfo *res;
	std::memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	std::stringstream ss;
	ss << port;
	std::string port_str = ss.str();

	int status = getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res);
	if (status != 0) {
		throw EventLoopException(gai_strerror(status));
	}

	int sd_listen = socket(res->ai_family, res->ai_socktype, 0);
	if (sd_listen < 0) {
		freeaddrinfo(res);
		throw EventLoopException(strerror(errno));
	}

	int yes = 1;
	if (setsockopt(sd_listen, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
		close(sd_listen);
		freeaddrinfo(res);
		throw EventLoopException(strerror(errno));
	}

	signal(SIGPIPE, SIG_IGN);
	if (bind(sd_listen, res->ai_addr, res->ai_addrlen) < 0) {
		close(sd_listen);
		freeaddrinfo(res);
		throw EventLoopException(strerror(errno));
	}

	freeaddrinfo(res);

	if (listen(sd_listen, SOMAXCONN) < 0) {
		close(sd_listen);
		throw EventLoopException(strerror(errno));
	}

	fcntl(sd_listen, F_SETFL, O_NONBLOCK);

	return sd_listen;
}
