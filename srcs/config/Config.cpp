#include "Config.hpp"
#include <fstream>
#include <sstream>

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

void Config::parseServerBlock(const std::vector<std::string>& tokens, std::size_t& i) {
	ServerContext server;
	bool index_specified = false;

	i++;
	if (i >= tokens.size() || tokens[i] != "{") {
		throw ConfigException("Error: Expected '{' after server directive.");
	}
	i++;
	while (i < tokens.size() && tokens[i] != "}") {
		if (tokens[i] == "listen") {
			handleListen(tokens, i, server);
		}
		else if (tokens[i] == "server_name") {
			handleServerName(tokens, i, server);
		}
		else if (tokens[i] == "root") {
			handleRoot(tokens, i, server);
		}
		else if (tokens[i] == "index") {
			handleIndex(tokens, i, server, index_specified);
		}
		else if (tokens[i] == "location") {
			parseLocationBlock(tokens, i, server);
		}
		else if (tokens[i] == "client_max_body_size") {
			handleClientMaxBodySize(tokens, i, server);
		}
		else if (tokens[i] == "error_page") {
			handleErrorPage(tokens, i, server);
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

void Config::parseLocationBlock(const std::vector<std::string>& tokens, std::size_t& i, ServerContext& server) {
	i++;
	if (i >= tokens.size()) {
		throw ConfigException("Error: Unexpected EOF in location block.");
	}

	LocationContext location(tokens[i]);
	bool index_specified = false;
	bool allowed_methods_specified = false;
	bool location_root_specified = false;
	bool alias_specified = false;
	bool redirect_specified = false;

	location.setRoot(server.getRoot());
	const std::vector<std::string>& parent_index = server.getIndex();
	for (std::size_t j = 0; j < parent_index.size(); ++j) {
		location.setIndex(parent_index[j]);
	}
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
			handleAllowMethods(tokens, i, location, allowed_methods_specified);
		}
		else if (tokens[i] == "root") {
			handleRoot(tokens, i, location);
			location_root_specified = true;
		}
		else if (tokens[i] == "alias") {
			handleAlias(tokens, i, location, alias_specified);
		}
		else if (tokens[i] == "index") {
			handleIndex(tokens, i, location, index_specified);
		}
		else if (tokens[i] == "autoindex") {
			handleAutoindex(tokens, i, location);
		}
		else if (tokens[i] == "return") {
			handleReturn(tokens, i, location, redirect_specified);
		}
		else if (tokens[i] == "upload_enable") {
			handleUploadEnable(tokens, i, location);
		}
		else if (tokens[i] == "upload_store") {
			handleUploadStore(tokens, i, location);
		}
		else if (tokens[i] == "cgi_extension") {
			handleCgiExtension(tokens, i, location);
		}
		else if (tokens[i] == "cgi_path") {
			handleCgiPath(tokens, i, location);
		}
		else if (tokens[i] == "client_max_body_size") {
			handleClientMaxBodySize(tokens, i, location);
		}
		else if (tokens[i] == "error_page") {
			handleErrorPage(tokens, i, location);
		}
		else {
			throw ConfigException("Error: Unknown location directive '" + tokens[i] + "'");
		}
		if (i < tokens.size() && tokens[i] == ";") {
			i++;
		} else if (i < tokens.size() && tokens[i] != "}") {
			throw ConfigException("Error: Expected ';' after location directive '" + tokens[i-1] + "'.");
		}
	}
	if (i < tokens.size() && tokens[i] == "}") {
		if (redirect_specified && (location_root_specified || alias_specified)) {
			throw ConfigException("Error: 'return' directive cannot be used with explicit 'root' or 'alias'.");
		}
		server.setLocation(location);
		return;
	}
	throw ConfigException("Error: Location block not closed with '}'.");
}

void Config::validateConfiguration() const {
	if (this->_servers.empty()) {
		throw ConfigException("Error: No server blocks found in configuration.");
	}
	for (std::size_t i = 0; i < this->_servers.size(); ++i) {
		this->_servers[i].validate();
	}
}

std::vector<std::string> Config::tokenize(const std::string& content) {
	std::vector<std::string> tokens;
	std::string current_token = "";

	for (std::size_t i = 0; i < content.length(); ++i) {
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
	for (std::size_t i = 0; i < tokens.size(); ++i) {
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

	for (std::size_t i = 0; i < this->_servers.size(); ++i) {
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
	std::size_t max_match_length = 0;
	const std::vector<LocationContext>& locations = server->getLocations();
	for (std::size_t i = 0; i < locations.size(); ++i) {
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
