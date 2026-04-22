#include "../../includes/config/Config.hpp"

Config::Config() {}

Config::~Config() {}

Config::Config(const Config& other) : _servers(other._servers) {}

Config& Config::operator=(const Config& other) {
	if (this != &other) {
		this->_servers = other._servers;
	}
	return *this;
}

const std::vector<ServerContext>& Config::getServers() const {
	return this->_servers;
}

bool Config::loadFile(const std::string& filename) {
  std::cout << "Loading: " << filename << std::endl;
  return true;
}

std::vector<std::string> Config::tokenize(const std::string& content) {
	std::vector<std::string> tokens;
	std::string current_token = "";

	for (size_t i = 0; i < content.length(); ++i) {
		char c = content[i];
		if (c == '#') {
			while (i < content.length() && content[i] != '\n') {
				i++;
			}
			continue;
		}
		if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
			if (!current_token.empty()) {
				tokens.push_back(current_token);
				current_token = "";
			}
			continue;
		}
		if (c == '{' || c == '}' || c == ';') {
			if (!current_token.empty()) {
				tokens.push_back(current_token);
				current_token = "";
			}
			tokens.push_back(std::string(1, c));
			continue;
		}
		current_token += c;
	}
	if (!current_token.empty()) {
		tokens.push_back(current_token);
	}
	return tokens;
}
