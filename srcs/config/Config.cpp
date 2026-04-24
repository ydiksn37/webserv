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

void Config::parseServerBlock(const std::vector<std::string>& tokens, size_t& i) {
	ServerContext server;

	i++;
	if (i >= tokens.size() || tokens[i] != "{") {
		throw ConfigException("Error: Expected '{' after server directive.");
	}
	i++;
	while (i < tokens.size() && tokens[i] != "}") {
		if (tokens[i] == "listen") {
			i++;
			if (i >= tokens.size()) {
				throw ConfigException("Error: Unexpected EOF after listen.");
			}
			std::string listen_val = tokens[i];
			size_t colon_pos = listen_val.find(':');
			if (colon_pos != std::string::npos) {
				std::string ip = listen_val.substr(0, colon_pos);
				std::string port_str = listen_val.substr(colon_pos + 1);
				server.setPort(static_cast<int>(parseLong(port_str)));
			} else {
				server.setPort(static_cast<int>(parseLong(listen_val)));
			}
			i++;
			if (i >= tokens.size() || tokens[i] != ";") {
				throw ConfigException("Error: Expected ';' after listen port.");
			}
		}
		else if (tokens[i] == "server_name") {
			i++;
			while (i < tokens.size() && tokens[i] != ";") {
				server.setServerName(tokens[i]);
				i++;
			}
			if (i >= tokens.size() || tokens[i] != ";") {
				throw ConfigException("Error: Expected ';' after server_name.");
			}
		}
		else if (tokens[i] == "location") {
			parseLocationBlock(tokens, i, server);
		}
		else if (tokens[i] == "client_max_body_size") {
			i++;
			if (i >= tokens.size()) {
				throw ConfigException("Error: Unexpected EOF after client_max_body_size.");
			}
			server.setClientMaxBodySize(std::strtoul(tokens[i].c_str(), NULL, 10));
			i++;
			if (i >= tokens.size() || tokens[i] != ";") {
				throw ConfigException("Error: Expected ';' after client_max_body_size.");
			}
		}
		else if (tokens[i] == "error_page") {
			i++;
			std::vector<int> codes;
			while (i < tokens.size() && tokens[i] != ";" && isdigit(tokens[i][0])) {
				codes.push_back(static_cast<int>(parseLong(tokens[i])));
				i++;
			}
			if (i >= tokens.size() || tokens[i] == ";") {
				throw ConfigException("Error: Expected status code for error_page.");
			}
			std::string path = tokens[i];
			for (size_t j = 0; j < codes.size(); ++j) {
				server.setErrorPage(codes[j], path);
			}
			i++;
			if (i >= tokens.size() || tokens[i] != ";") {
				throw ConfigException("Error: Expected ';' after error_page.");
			}
		}
		else {
			throw ConfigException("Error: Unknown server directive '" + tokens[i] + "'");
		}
		i++;
	}
	if (i < tokens.size() && tokens[i] == "}") {
		this->_servers.push_back(server);
		return;
	}
	throw ConfigException("Error: Server block not closed with '}'.");
}

void Config::parseLocationBlock(const std::vector<std::string>& tokens, size_t& i, ServerContext& server) {
	i++;
	if (i >= tokens.size()) {
		throw ConfigException("Error: Unexpected EOF in location block.");
	}

	LocationContext location(tokens[i]);

	location.setClientMaxBodySize(server.getClientMaxBodySize());
	const std::map<int, std::string>& parent_errors = server.getErrorPages();
	for (std::map<int, std::string>::const_iterator it = parent_errors.begin(); it != parent_errors.end(); ++it) {
		location.setErrorPage(it->first, it->second);
	}

	i++;
	if (i >= tokens.size() || tokens[i] != "{") {
		throw ConfigException("Error: Expected '{' after location path.");
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
			int code = static_cast<int>(parseLong(tokens[i]));
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
			if (i >= tokens.size()) {
				throw ConfigException("Error: Unexpected EOF in cgi_path.");
			}
			std::string ext = tokens[i];
			i++;
			if (i >= tokens.size()) {
				throw ConfigException("Error: Unexpected EOF in cgi_path.");
			}
			std::string path = tokens[i];
			location.setCgiPath(ext, path);
			i++;
		}
		else if (tokens[i] == "client_max_body_size") {
			i++;
			if (i >= tokens.size()) {
				throw ConfigException("Error: Unexpected EOF in client_max_body_size.");
			}
			location.setClientMaxBodySize(std::strtoul(tokens[i].c_str(), NULL, 10));
			i++;
		}
		else if (tokens[i] == "error_page") {
			i++;
			std::vector<int> codes;
			while (i < tokens.size() && tokens[i] != ";" && isdigit(tokens[i][0])) {
				codes.push_back(static_cast<int>(parseLong(tokens[i])));
				i++;
			}
			if (i >= tokens.size() || tokens[i] == ";") {
				throw ConfigException("Error: Expected status code for error_page.");
			}
			std::string path = tokens[i];
			for (size_t j = 0; j < codes.size(); ++j) {
				location.setErrorPage(codes[j], path);
			}
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
		} else if (i < tokens.size() && tokens[i] != "}") {
			throw ConfigException("Error: Expected ';' after location directive.");
		}
	}
	if (i < tokens.size() && tokens[i] == "}") {
		server.setLocation(location);
		return;
	}
	throw ConfigException("Error: Location block not closed with '}'.");
}

void Config::validateConfiguration() const {
	if (this->_servers.empty()) {
		throw ConfigException("Error: No server blocks found in configuration.");
	}
	for (size_t i = 0; i < this->_servers.size(); ++i) {
		this->_servers[i].validate();
	}
}

long Config::parseLong(const std::string& str) const {
	char* endptr;
	errno = 0;
	long val = std::strtol(str.c_str(), &endptr, 10);

	if (errno == ERANGE) {
		throw ConfigException("Error: Number out of range: " + str);
	}
	if (*endptr != '\0') {
		throw ConfigException("Error: Invalid number format: " + str);
	}
	return val;
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

void Config::loadFile(const std::string& filename) {
	std::ifstream file(filename.c_str());
	std::stringstream buffer;
	std::vector<std::string> tokens;

	if (!file.is_open()) {
		throw ConfigException("Error: Cannot open configuration file: " + filename);
	}
	buffer << file.rdbuf();
	tokens = tokenize(buffer.str());
	if (tokens.empty()) {
		throw ConfigException("Error: Configuration file is empty.");
	}
	for (size_t i = 0; i < tokens.size(); ++i) {
		if (tokens[i] == "server") {
			parseServerBlock(tokens, i);
		} else {
			throw ConfigException("Error: Expected 'server' block, found '" + tokens[i] + "'");
		}
	}
	validateConfiguration();
}

const ServerContext* Config::getServer(int port, const std::string& host_name) const {
	const ServerContext* default_server = NULL;

	for (size_t i = 0; i < this->_servers.size(); ++i) {
		if (this->_servers[i].getPort() == port) {
			if (default_server == NULL) {
				default_server = &this->_servers[i];
			}
			if (this->_servers[i].getIsServerNameIncluded(host_name)) {
				return &this->_servers[i];
			}
		}
	}
	return default_server;
}

const LocationContext* Config::matchLocation(const ServerContext* server, const std::string& uri) const {
	if (server == NULL) {
		return NULL;
	}

	const LocationContext* best_match = NULL;
	size_t max_match_length = 0;
	const std::vector<LocationContext>& locations = server->getLocations();
	for (size_t i = 0; i < locations.size(); ++i) {
		const std::string& path = locations[i].getPath();
		if (uri.find(path) == 0) {
			bool is_boundary_safe = (path == "/" || uri.length() == path.length() || uri[path.length()] == '/');
			if (is_boundary_safe) {
				if (path.length() > max_match_length) {
					max_match_length = path.length();
					best_match = &locations[i];
				}
			}
		}
	}
	return best_match;
}
