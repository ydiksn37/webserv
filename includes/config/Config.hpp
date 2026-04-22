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
		bool loadFile(const std::string& filename);
		const std::vector<ServerContext>& getServers() const;
};

#endif
