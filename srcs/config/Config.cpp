#include "../../includes/config/Config.hpp"

Config::Config() : _servers() {}

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
			std::string listen_val = tokens[i];
			size_t colon_pos = listen_val.find(':');
			if (colon_pos != std::string::npos) {
				std::string ip = listen_val.substr(0, colon_pos);
				std::string port_str = listen_val.substr(colon_pos + 1);
				server.setPort(std::atoi(port_str.c_str()));
			} else {
				server.setPort(std::atoi(listen_val.c_str()));
			}
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
		else if (tokens[i] == "location") {
			if (!parseLocationBlock(tokens, i, server)) {
				return false;
			}
		}
		else if (tokens[i] == "client_max_body_size") {
			i++;
			if (i >= tokens.size()) {
				return false;
			}
			server.setClientMaxBodySize(std::strtoul(tokens[i].c_str(), NULL, 10));
			i++;
			if (i >= tokens.size() || tokens[i] != ";") {
				std::cerr << "Error: Expected ';' after client_max_body_size" << std::endl;
				return false;
			}
		}
		else if (tokens[i] == "error_page") {
			i++;
			std::vector<int> codes;
			while (i < tokens.size() && tokens[i] != ";" && isdigit(tokens[i][0])) {
				codes.push_back(std::atoi(tokens[i].c_str()));
				i++;
			}
			if (i >= tokens.size() || tokens[i] == ";") {
				return false;
			}
			std::string path = tokens[i];
			for (size_t j = 0; j < codes.size(); ++j) {
				server.setErrorPage(codes[j], path);
			}
			i++;
			if (i >= tokens.size() || tokens[i] != ";") {
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

bool Config::parseLocationBlock(const std::vector<std::string>& tokens, size_t& i, ServerContext& server) {
	i++;
	if (i >= tokens.size()) {
		return false;
	}

	LocationContext location(tokens[i]);

	i++;
	if (i >= tokens.size() || tokens[i] != "{") {
		return false;
	}
	i++;
	while (i < tokens.size() && tokens[i] != "}") {
		if (tokens[i] == "allow_methods" || tokens[i] == "allow_method") {
			i++;
			while (i < tokens.size() && tokens[i] != ";") {
				location.setAllowedMethod(tokens[i]);
				i++;
			}
		}
		else if (tokens[i] == "root") {
			i++;
			location.setRoot(tokens[i]);
			i++;
		}
		else if (tokens[i] == "alias") {
			i++;
			location.setAlias(tokens[i]);
			i++;
		}
		else if (tokens[i] == "index") {
			i++;
			while (i < tokens.size() && tokens[i] != ";") {
				location.setIndex(tokens[i]);
				i++;
			}
		}
		else if (tokens[i] == "autoindex") {
			i++;
			location.setAutoindex(tokens[i] == "on");
			i++;
		}
		else if (tokens[i] == "return") {
			i++;
			int code = std::atoi(tokens[i].c_str());
			i++;
			location.setRedirect(code, tokens[i]);
			i++;
		}
		else if (tokens[i] == "upload_enable") {
			i++;
			location.setUploadEnable(tokens[i] == "on");
			i++;
		}
		else if (tokens[i] == "upload_store") {
			i++;
			location.setUploadStore(tokens[i]);
			i++;
		}
		else if (tokens[i] == "cgi_extension") {
			i++;
			while (i < tokens.size() && tokens[i] != ";") {
				i++;
			}
		}
		else if (tokens[i] == "cgi_path") {
			i++;
			if (i >= tokens.size()) return false;
			std::string ext = tokens[i];
			i++;
			if (i >= tokens.size()) return false;
			std::string path = tokens[i];
			
			location.setCgiPath(ext, path);
			i++;
		}
		else {
			std::cerr << "Warning: Unknown location directive '" << tokens[i] << "'" << std::endl;
			while (i < tokens.size() && tokens[i] != ";" && tokens[i] != "}") {
				i++;
			}
		}
		if (i < tokens.size() && tokens[i] == ";") {
			i++;
		}
	}
	if (i < tokens.size() && tokens[i] == "}") {
		server.setLocation(location);
		return true;
	}
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
