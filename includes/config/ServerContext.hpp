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
		ServerContext();
		~ServerContext();
		ServerContext(const ServerContext& other);
		ServerContext& operator=(const ServerContext& other);

		void setPort(int port);
		void setServerName(const std::string& name);
		int getPort() const;
		const std::string& getServerName() const;
};

#endif
