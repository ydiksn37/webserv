#include "ServerContext.hpp"

ServerContext::ServerContext()
	: _port(80),
		_server_names(),
		_client_max_body_size(1048576),
		_error_pages(),
		_root(""),
		_index(),
		_locations() {}

ServerContext::~ServerContext() {}

ServerContext::ServerContext(const ServerContext& other)
	: _port(other._port), 
		_server_names(other._server_names),
		_client_max_body_size(other._client_max_body_size),
		_error_pages(other._error_pages),
		_root(other._root),
		_index(other._index),
		_locations(other._locations) {}

ServerContext& ServerContext::operator=(const ServerContext& other) {
	if (this != &other) {
		this->_port = other._port;
		this->_server_names = other._server_names;
		this->_client_max_body_size = other._client_max_body_size;
		this->_error_pages = other._error_pages;
		this->_root = other._root;
		this->_index = other._index;
		this->_locations = other._locations;
	}
	return *this;
}

void ServerContext::setPort(int port) {
	this->_port = port;
}

void ServerContext::setServerName(const std::string& name) {
	this->_server_names.insert(name);
}

void ServerContext::setClientMaxBodySize(size_t size) {
	this->_client_max_body_size = size;
}

void ServerContext::setErrorPage(int status_code, const std::string& path) {
	this->_error_pages[status_code] = path;
}

void ServerContext::setRoot(const std::string& root) {
	this->_root = root;
}

void ServerContext::setIndex(const std::string& index) {
	this->_index.push_back(index);
}

void ServerContext::clearIndex() {
	this->_index.clear();
}

void ServerContext::setLocation(const LocationContext& location) {
	this->_locations.push_back(location);
}

int ServerContext::getPort() const{
	return this->_port;
}

const std::set<std::string>& ServerContext::getServerNames() const {
	return this->_server_names;
}

bool ServerContext::getIsServerNameIncluded(const std::string& name) const {
	return this->_server_names.find(name) != this->_server_names.end();
}

size_t ServerContext::getClientMaxBodySize() const {
	return this->_client_max_body_size;
}

const std::map<int, std::string>& ServerContext::getErrorPages() const {
	return this->_error_pages;
}

const std::string& ServerContext::getRoot() const {
	return this->_root;
}

const std::vector<std::string>& ServerContext::getIndex() const {
	if (this->_index.empty()) {
		static std::vector<std::string> default_index;
		if (default_index.empty()) {
			default_index.push_back("index.html");
		}
		return default_index;
	}
	return this->_index;
}

const std::vector<LocationContext>& ServerContext::getLocations() const {
	return this->_locations;
}

void ServerContext::validate() const {
	if (this->_port <= 0 || this->_port > 65535) {
		throw std::runtime_error("Invalid or missing 'listen' port in server block.");
	}
	if (this->_locations.empty()) {
		throw std::runtime_error("Server must contain at least one 'location' block.");
	}
	for (size_t i = 0; i < this->_locations.size(); ++i) {
		this->_locations[i].validate();
	}
}
