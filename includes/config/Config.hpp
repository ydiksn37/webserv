#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <iostream>
# include <string>
# include <vector>
# include "ServerContext.hpp"

class Config {
	private:
		std::vector<ServerContext> _servers;

	public:
		Config();
		~Config();
		Config(const Config& other);
		Config& operator=(const Config& other);

		const std::vector<ServerContext>& getServers() const;
		bool loadFile(const std::string& filename);
		std::vector<std::string> tokenize(const std::string& content);
};

#endif
