#include "Config.hpp"

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
	bool index_specified = false;

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
			long port;
			if (colon_pos != std::string::npos) {
				std::string port_str = listen_val.substr(colon_pos + 1);
				if (port_str.empty()) {
					throw ConfigException("Error: Invalid listen format.");
				}
				port = parseLong(port_str);
			}
			else {
				port = parseLong(listen_val);
			}
			if (port <= 0 || port > 65535) {
				throw ConfigException("Error: Port out of range: " + tokens[i]);
			}
			server.setPort(static_cast<int>(port));
			i++;
			if (i >= tokens.size() || tokens[i] != ";") {
				throw ConfigException("Error: Expected ';' after listen.");
			}
		}
		else if (tokens[i] == "server_name") {
			i++;
			if (i >= tokens.size() || tokens[i] == ";") {
				throw ConfigException("Error: server_name requires at least one argument.");
			}
			while (i < tokens.size() && tokens[i] != ";") {
				server.setServerName(tokens[i]);
				i++;
			}
			if (i >= tokens.size() || tokens[i] != ";") {
				throw ConfigException("Error: Expected ';' after server_name.");
			}
		}
		else if (tokens[i] == "root") {
			i++;
			if (i >= tokens.size() || tokens[i] == ";") {
				throw ConfigException("Error: root requires an argument.");
			}
			server.setRoot(tokens[i]);
			i++;
			if (i >= tokens.size() || tokens[i] != ";") {
				throw ConfigException("Error: Expected ';' after root.");
			}
		}
		else if (tokens[i] == "index") {
			i++;
			if (i >= tokens.size() || tokens[i] == ";") {
				throw ConfigException("Error: index requires at least one argument.");
			}
			if (!index_specified) {
				server.clearIndex();
				index_specified = true;
			}
			while (i < tokens.size() && tokens[i] != ";") {
				server.setIndex(tokens[i]);
				i++;
			}
			if (i >= tokens.size() || tokens[i] != ";") {
				throw ConfigException("Error: Expected ';' after index.");
			}
		}
		else if (tokens[i] == "location") {
			parseLocationBlock(tokens, i, server);
		}
		else if (tokens[i] == "client_max_body_size") {
			i++;
			if (i >= tokens.size() || tokens[i] == ";") {
				throw ConfigException("Error: client_max_body_size requires an argument.");
			}
			server.setClientMaxBodySize(parseUnsignedLong(tokens[i]));
			i++;
			if (i >= tokens.size() || tokens[i] != ";") {
				throw ConfigException("Error: Expected ';' after client_max_body_size.");
			}
		}
		else if (tokens[i] == "error_page") {
			i++;
			std::vector<int> codes;
			while (i < tokens.size() && tokens[i] != ";" && std::isdigit(tokens[i][0])) {
				long code = parseLong(tokens[i]);
				if (code < 300 || code > 599) {
					throw ConfigException("Error: Invalid error code: " + tokens[i]);
				}
				codes.push_back(static_cast<int>(code));
				i++;
			}
			if (codes.empty()) {
				throw ConfigException("Error: error_page requires at least one status code.");
			}
			if (i >= tokens.size() || tokens[i] == ";") {
				throw ConfigException("Error: error_page requires a destination path.");
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
	bool index_specified = false;
	bool allowed_methods_specified = false;
	bool location_root_specified = false;
	bool alias_specified = false;
	bool redirect_specified = false;

	location.setRoot(server.getRoot());
	const std::vector<std::string>& parent_index = server.getIndex();
	for (size_t j = 0; j < parent_index.size(); ++j) {
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
			i++;
			if (i >= tokens.size() || tokens[i] == ";") {
				throw ConfigException("Error: allow_methods requires at least one argument.");
			}
			if (!allowed_methods_specified) {
				location.clearAllowedMethods();
				allowed_methods_specified = true;
			}
			while (i < tokens.size() && tokens[i] != ";") {
				if (tokens[i] != "GET" && tokens[i] != "POST" && tokens[i] != "DELETE") {
					throw ConfigException("Error: Invalid or unsupported HTTP method '" + tokens[i] + "' in allow_methods.");
				}
				location.setAllowedMethod(tokens[i]);
				i++;
			}
		}
		else if (tokens[i] == "root") {
			i++;
			if (i >= tokens.size() || tokens[i] == ";") {
				throw ConfigException("Error: root requires an argument.");
			}
			location.setRoot(tokens[i]);
			location_root_specified = true;
			i++;
		}
		else if (tokens[i] == "alias") {
			i++;
			if (i >= tokens.size() || tokens[i] == ";") {
				throw ConfigException("Error: alias requires an argument.");
			}
			location.setAlias(tokens[i]);
			alias_specified = true;
			i++;
		}
		else if (tokens[i] == "index") {
			i++;
			if (i >= tokens.size() || tokens[i] == ";") {
				throw ConfigException("Error: index requires at least one argument.");
			}
			if (!index_specified) {
				location.clearIndex();
				index_specified = true;
			}
			while (i < tokens.size() && tokens[i] != ";") {
				location.setIndex(tokens[i]);
				i++;
			}
		}
		else if (tokens[i] == "autoindex") {
			i++;
			if (i >= tokens.size() || tokens[i] == ";") {
				throw ConfigException("Error: autoindex requires 'on' or 'off'.");
			}
			if (tokens[i] != "on" && tokens[i] != "off") {
				throw ConfigException("Error: autoindex must be 'on' or 'off'.");
			}
			location.setAutoindex(tokens[i] == "on");
			i++;
		}
		else if (tokens[i] == "return") {
			i++;
			if (i >= tokens.size() || tokens[i] == ";") {
				throw ConfigException("Error: return requires a status code and URL.");
			}
			int code = static_cast<int>(parseLong(tokens[i]));
			if (code < 300 || code > 599) {
				throw ConfigException("Error: Invalid redirect code: " + tokens[i]);
			}
			i++;
			if (i >= tokens.size() || tokens[i] == ";") {
				throw ConfigException("Error: return requires a URL.");
			}
			location.setRedirect(code, tokens[i]);
			redirect_specified = true;
			i++;
		}
		else if (tokens[i] == "upload_enable") {
			i++;
			if (i >= tokens.size() || tokens[i] == ";") {
				throw ConfigException("Error: upload_enable requires 'on' or 'off'.");
			}
			if (tokens[i] != "on" && tokens[i] != "off") {
				throw ConfigException("Error: upload_enable must be 'on' or 'off'.");
			}
			location.setUploadEnable(tokens[i] == "on");
			i++;
		}
		else if (tokens[i] == "upload_store") {
			i++;
			if (i >= tokens.size() || tokens[i] == ";") {
				throw ConfigException("Error: upload_store requires a path.");
			}
			location.setUploadStore(tokens[i]);
			i++;
		}
		else if (tokens[i] == "cgi_extension") {
			i++;
			if (i >= tokens.size() || tokens[i] == ";") {
				throw ConfigException("Error: cgi_extension requires at least one argument.");
			}
			while (i < tokens.size() && tokens[i] != ";") {
				if (tokens[i][0] != '.') {
					throw ConfigException("Error: CGI extension must start with a dot: " + tokens[i]);
				}
				location.setCgiExtension(tokens[i]);
				i++;
			}
		}
		else if (tokens[i] == "cgi_path") {
			i++;
			if (i >= tokens.size() || tokens[i] == ";") {
				throw ConfigException("Error: cgi_path requires an extension and a path.");
			}
			std::string ext = tokens[i];
			i++;
			if (i >= tokens.size() || tokens[i] == ";") {
				throw ConfigException("Error: cgi_path requires a path.");
			}
			std::string path = tokens[i];
			location.setCgiPath(ext, path);
			i++;
		}
		else if (tokens[i] == "client_max_body_size") {
			i++;
			if (i >= tokens.size() || tokens[i] == ";") {
				throw ConfigException("Error: client_max_body_size requires an argument.");
			}
			location.setClientMaxBodySize(parseUnsignedLong(tokens[i]));
			i++;
		}
		else if (tokens[i] == "error_page") {
			i++;
			std::vector<int> codes;
			while (i < tokens.size() && tokens[i] != ";" && std::isdigit(tokens[i][0])) {
				long code = parseLong(tokens[i]);
				if (code < 300 || code > 599) {
					throw ConfigException("Error: Invalid error code: " + tokens[i]);
				}
				codes.push_back(static_cast<int>(code));
				i++;
			}
			if (codes.empty()) {
				throw ConfigException("Error: error_page requires at least one status code.");
			}
			if (i >= tokens.size() || tokens[i] == ";") {
				throw ConfigException("Error: error_page requires a destination path.");
			}
			std::string path = tokens[i];
			for (size_t j = 0; j < codes.size(); ++j) {
				location.setErrorPage(codes[j], path);
			}
			i++;
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

unsigned long Config::parseUnsignedLong(const std::string& str) const {
	if (str.empty() || !std::isdigit(str[0])) {
		throw ConfigException("Error: Invalid unsigned number format: " + str);
	}
	char* endptr;
	errno = 0;
	unsigned long val = std::strtoul(str.c_str(), &endptr, 10);

	if (errno == ERANGE) {
		throw ConfigException("Error: Number out of range: " + str);
	}
	if (*endptr != '\0') {
		std::string suffix = endptr;
		if (suffix == "k" || suffix == "K") {
			val *= 1024;
		} else if (suffix == "m" || suffix == "M") {
			val *= 1024 * 1024;
		} else if (suffix == "g" || suffix == "G") {
			val *= 1024 * 1024 * 1024;
		} else {
			throw ConfigException("Error: Invalid unsigned number format: " + str);
		}
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
