#ifndef SERVERCONTEXT__HPP
# define SERVERCONTEXT__HPP

# include <iostream>
# include <string>
# include <vector>

class ServerContext {
	private:
		int         _port;
		std::string _server_name;
	public:
		ServerContext() : _port(80), _server_name("") {}
		void setPort(int port) { _port = port; }
		void setServerName(const std::string& name) { _server_name = name; }
		int getPort() const { return _port; }
		const std::string& getServerName() const { return _server_name; }
};

#endif
