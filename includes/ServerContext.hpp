#ifndef SERVERCONTEXT_HPP
# define SERVERCONTEXT_HPP

# include "LocationContext.hpp"
# include <string>
# include <vector>
# include <map>
# include <set>

class ServerContext {
	public:
		ServerContext();
		~ServerContext();
		ServerContext(const ServerContext& other);
		ServerContext& operator=(const ServerContext& other);

		void setPort(int port);
		void setServerName(const std::string& name);
		void setClientMaxBodySize(size_t size);
		void setErrorPage(int status_code, const std::string& path);
		void setRoot(const std::string& root);
		void setIndex(const std::string& index);
		void clearIndex();
		void setLocation(const LocationContext& location);

		int																	getPort() const;
		const std::set<std::string>&				getServerNames() const;
		bool																getIsServerNameIncluded(const std::string& name) const;
		size_t															getClientMaxBodySize() const;
		const std::map<int, std::string>&		getErrorPages() const;
		const std::string&									getRoot() const;
		const std::vector<std::string>&			getIndex() const;
		const std::vector<LocationContext>&	getLocations() const;

		void validate() const;

	private:
		int														_port;
		std::set<std::string>					_server_names;
		size_t												_client_max_body_size;
		std::map<int, std::string>		_error_pages;
		std::string										_root;
		std::vector<std::string>			_index;
		std::vector<LocationContext>	_locations;
};

#endif
