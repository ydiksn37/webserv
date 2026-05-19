#include "Config.hpp"

void Config::handleListen(const std::vector<std::string>& tokens, std::size_t& i, ServerContext& server) {
	i++;
	if (i >= tokens.size()) {
		throw ConfigException("Error: Unexpected EOF after listen.");
	}
	std::string listen_val = tokens[i];
	std::size_t colon_pos = listen_val.find(':');
	long port;
	if (colon_pos != std::string::npos) {
		std::string host = listen_val.substr(0, colon_pos);
		if (!host.empty()) {
			server.setHost(host);
		}
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

void Config::handleServerName(const std::vector<std::string>& tokens, std::size_t& i, ServerContext& server) {
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

void Config::handleRoot(const std::vector<std::string>& tokens, std::size_t& i, ServerContext& server) {
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

void Config::handleRoot(const std::vector<std::string>& tokens, std::size_t& i, LocationContext& location) {
	i++;
	if (i >= tokens.size() || tokens[i] == ";") {
		throw ConfigException("Error: root requires an argument.");
	}
	location.setRoot(tokens[i]);
	i++;
}

void Config::handleIndex(const std::vector<std::string>& tokens, std::size_t& i, ServerContext& server, bool& index_specified) {
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

void Config::handleIndex(const std::vector<std::string>& tokens, std::size_t& i, LocationContext& location, bool& index_specified) {
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

void Config::handleClientMaxBodySize(const std::vector<std::string>& tokens, std::size_t& i, ServerContext& server) {
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

void Config::handleClientMaxBodySize(const std::vector<std::string>& tokens, std::size_t& i, LocationContext& location) {
	i++;
	if (i >= tokens.size() || tokens[i] == ";") {
		throw ConfigException("Error: client_max_body_size requires an argument.");
	}
	location.setClientMaxBodySize(parseUnsignedLong(tokens[i]));
	i++;
}

void Config::handleErrorPage(const std::vector<std::string>& tokens, std::size_t& i, ServerContext& server) {
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
	for (std::size_t j = 0; j < codes.size(); ++j) {
		server.setErrorPage(codes[j], path);
	}
	i++;
	if (i >= tokens.size() || tokens[i] != ";") {
		throw ConfigException("Error: Expected ';' after error_page.");
	}
}

void Config::handleErrorPage(const std::vector<std::string>& tokens, std::size_t& i, LocationContext& location) {
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
	for (std::size_t j = 0; j < codes.size(); ++j) {
		location.setErrorPage(codes[j], path);
	}
	i++;
}

void Config::handleAllowMethods(const std::vector<std::string>& tokens, std::size_t& i, LocationContext& location, bool& allowed_methods_specified) {
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

void Config::handleAlias(const std::vector<std::string>& tokens, std::size_t& i, LocationContext& location, bool& alias_specified) {
	i++;
	if (i >= tokens.size() || tokens[i] == ";") {
		throw ConfigException("Error: alias requires an argument.");
	}
	location.setAlias(tokens[i]);
	alias_specified = true;
	i++;
}

void Config::handleAutoindex(const std::vector<std::string>& tokens, std::size_t& i, LocationContext& location) {
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

void Config::handleReturn(const std::vector<std::string>& tokens, std::size_t& i, LocationContext& location, bool& redirect_specified) {
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

void Config::handleUploadEnable(const std::vector<std::string>& tokens, std::size_t& i, LocationContext& location) {
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

void Config::handleUploadStore(const std::vector<std::string>& tokens, std::size_t& i, LocationContext& location) {
	i++;
	if (i >= tokens.size() || tokens[i] == ";") {
		throw ConfigException("Error: upload_store requires a path.");
	}
	location.setUploadStore(tokens[i]);
	i++;
}

void Config::handleCgiExtension(const std::vector<std::string>& tokens, std::size_t& i, LocationContext& location) {
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

void Config::handleCgiPath(const std::vector<std::string>& tokens, std::size_t& i, LocationContext& location) {
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
