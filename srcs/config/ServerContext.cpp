#include "../../includes/config/ServerContext.hpp"

ServerContext::ServerContext() : _port(80), _server_name("") {}

ServerContext::~ServerContext() {}

ServerContext::ServerContext(const ServerContext& other) : _port(other._port), _server_name(other._server_name) {}

ServerContext& ServerContext::operator=(const ServerContext& other) {
	if (this != &other) {
		this->_port = other._port;
		this->_server_name = other._server_name;
	}
	return *this;
} 

void ServerContext::setPort(int port) {
	this->_port = port;
}

void ServerContext::setServerName(const std::string& name) {
	this->_server_name = name;
}

int ServerContext::getPort() const{
	return this->_port;
}

const std::string& ServerContext::getServerName() const {
	return this->_server_name;
}
