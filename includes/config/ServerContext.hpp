#ifndef SERVERCONTEXT_HPP
# define SERVERCONTEXT_HPP

# include <iostream>
# include <string>
# include <vector>
# include <map>
# include "LocationContext.hpp"

class ServerContext {
	private:
		int                          _port;
		std::string                  _server_name;
		size_t                       _client_max_body_size;
		std::map<int, std::string>   _error_pages;
		std::vector<LocationContext> _locations;

	public:
		ServerContext();
		~ServerContext();
		ServerContext(const ServerContext& other);
		ServerContext& operator=(const ServerContext& other);

		void setPort(int port);
		void setServerName(const std::string& name);
		void setClientMaxBodySize(size_t size);
		void setErrorPage(int status_code, const std::string& path);
		void setLocation(const LocationContext& location);

		int getPort() const;
		const std::string& getServerName() const;
		size_t getClientMaxBodySize() const;
		const std::map<int, std::string>& getErrorPages() const;
		const std::vector<LocationContext>& getLocations() const;
};

#endif
