#include "../../includes/config/Config.hpp"

Config::Config() {}

Config::~Config() {}

bool Config::loadFile(const std::string& filename) {
  std::cout << "Loading: " << filename << std::endl;
  return true;
}

const std::vector<ServerContext>& Config::getServers() const {
	return this->_servers;
}
