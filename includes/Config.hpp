#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <iostream>
# include <string>
# include <vector>
# include <fstream>
# include <sstream>
# include <cstdlib>
# include <stdexcept>
# include <cctype>
# include <cerrno>
# include "../srcs/config/ServerContext.hpp"

class Config {
	private:
		std::vector<ServerContext>	_servers;
		void					parseServerBlock(const std::vector<std::string>& tokens, size_t& i);
		void					parseLocationBlock(const std::vector<std::string>& tokens, size_t& i, ServerContext& server);
		void					validateConfiguration() const;
		long					parseLong(const std::string& str) const;
		unsigned long	parseUnsignedLong(const std::string& str) const;

	public:
		Config();
		~Config();
		Config(const Config& other);
		Config& operator=(const Config& other);

		const std::vector<ServerContext>&	getServers() const;

		std::vector<std::string>	tokenize(const std::string& content);
		void											loadFile(const std::string& filename);
		const ServerContext*			getServer(int port, const std::string& host_name) const;
		const LocationContext*		matchLocation(const ServerContext* server, const std::string& uri) const;

		class ConfigException : public std::runtime_error {
			public:
				ConfigException(const std::string& msg) : std::runtime_error(msg) {}
		};
};

#endif
