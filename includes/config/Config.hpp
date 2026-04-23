#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <iostream>
# include <string>
# include <vector>
# include <fstream>
# include <sstream>
# include <cstdlib>
# include "ServerContext.hpp"

class Config {
	private:
		std::vector<ServerContext> _servers;
		bool parseServerBlock(const std::vector<std::string>& tokens, size_t& i);

	public:
		Config();
		~Config();
		Config(const Config& other);
		Config& operator=(const Config& other);

		const std::vector<ServerContext>& getServers() const;
		std::vector<std::string> tokenize(const std::string& content);
		bool loadFile(const std::string& filename);
};

#endif
