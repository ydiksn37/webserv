#include "../../includes/config/LocationContext.hpp"

LocationContext::LocationContext() : _path("") {}

LocationContext::~LocationContext() {}

LocationContext::LocationContext(const LocationContext& other) : _path(other._path) {}

LocationContext& LocationContext::operator=(const LocationContext& other) {
	if (this != &other) {
		this->_path = other._path;
	}
	return *this;
}

LocationContext::LocationContext(const std::string& path) : _path(path) {}

void LocationContext::setPath(const std::string& path) {
	this->_path = path;
}

const std::string& LocationContext::getPath() const {
	return this->_path;
}
