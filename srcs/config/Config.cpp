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

bool Config::parseServerBlock(const std::vector<std::string>& tokens, size_t& i) {
	ServerContext server;

	i++;
	if (i >= tokens.size() || tokens[i] != "{") {
		std::cerr << "Error: Expected '{' after server" << std::endl;
		return false;
	}
	i++;
	while (i < tokens.size() && tokens[i] != "}") {
		if (tokens[i] == "listen") {
			i++;
			if (i >= tokens.size()) {
				return false;
			}
			server.setPort(std::atoi(tokens[i].c_str()));
			i++;
			if (i >= tokens.size() || tokens[i] != ";") {
				std::cerr << "Error: Expected ';' after listen port" << std::endl;
				return false;
			}
		}
		else if (tokens[i] == "server_name") {
			i++;
			if (i >= tokens.size()) {
				return false;
			}
			server.setServerName(tokens[i]);
			i++;
			if (i >= tokens.size() || tokens[i] != ";") {
				std::cerr << "Error: Expected ';' after server_name" << std::endl;
				return false;
			}
		}
		else {
			std::cerr << "Error: Unknown directive '" << tokens[i] << "'" << std::endl;
			return false;
		}
		i++;
	}
	if (i < tokens.size() && tokens[i] == "}") {
		this->_servers.push_back(server);
		return true;
	}
	std::cerr << "Error: Server block not closed with '}'" << std::endl;
	return false;
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

bool Config::loadFile(const std::string& filename) {
	std::ifstream file(filename.c_str());
	std::stringstream buffer;
	std::vector<std::string> tokens;

	if (!file.is_open()) {
		std::cerr << "Error: Cannot open file " << filename << std::endl;
		return false;
	}
	buffer << file.rdbuf();
	tokens = tokenize(buffer.str());
	for (size_t i = 0; i < tokens.size(); ++i) {
		if (tokens[i] == "server") {
			if (!parseServerBlock(tokens, i)) {
				return false;
			}
		} else {
			std::cerr << "Error: Expected 'server' block, found '" << tokens[i] << "'" << std::endl;
			return false;
		}
	}
	return true;
}
