#include "../../includes/config/LocationContext.hpp"

LocationContext::LocationContext()
	: _path(""),
		_allowed_methods(0),
		_root(""),
		_alias(""),
		_autoindex(false),
		_redirect_code(0),
		_redirect_url(""),
		_upload_enable(false),
		_upload_store("") {}

LocationContext::~LocationContext() {}

LocationContext::LocationContext(const LocationContext& other)
	: _path(other._path),
		_allowed_methods(other._allowed_methods),
		_root(other._root),
		_alias(other._alias),
		_autoindex(other._autoindex),
		_redirect_code(other._redirect_code),
		_redirect_url(other._redirect_url),
		_upload_enable(other._upload_enable),
		_upload_store(other._upload_store) {}

LocationContext& LocationContext::operator=(const LocationContext& other) {
	if (this != &other) {
		this->_path = other._path;
		this->_allowed_methods = other._allowed_methods;
		this->_root = other._root;
		this->_alias = other._alias;
		this->_autoindex = other._autoindex;
		this->_redirect_code = other._redirect_code;
		this->_redirect_url = other._redirect_url;
		this->_upload_enable = other._upload_enable;
		this->_upload_store = other._upload_store;
	}
	return *this;
}

LocationContext::LocationContext(const std::string& path)
	: _path(path),
		_allowed_methods(0),
		_root(""),
		_alias(""),
		_autoindex(false),
		_redirect_code(0),
		_redirect_url(""),
		_upload_enable(false),
		_upload_store("") {}

void LocationContext::setPath(const std::string& path) {
	this->_path = path;
}

void LocationContext::setAllowedMethod(const std::string& method) {
	if (method == "GET") {
		_allowed_methods |= METHOD_GET;
	} else if (method == "POST") {
		_allowed_methods |= METHOD_POST;
	} else if (method == "DELETE") {
		_allowed_methods |= METHOD_DELETE;
	}
}

void LocationContext::setRoot(const std::string& root) {
	this->_root = root;
}

void LocationContext::setAlias(const std::string& alias) {
	this->_alias = alias;
}

void LocationContext::setIndex(const std::string& index) {
	this->_index.push_back(index);
}

void LocationContext::setAutoindex(bool autoindex) {
	this->_autoindex = autoindex;
}

void LocationContext::setRedirect(int code, const std::string& url) {
	this->_redirect_code = code;
	this->_redirect_url = url;
}

void LocationContext::setUploadEnable(bool enable) {
	this->_upload_enable = enable;
}

void LocationContext::setUploadStore(const std::string& store) {
	this->_upload_store = store;
}

void LocationContext::setCgiPath(const std::string& ext, const std::string& path) {
	this->_cgi_info[ext] = path;
}

const std::string& LocationContext::getPath() const {
	return this->_path;
}

bool LocationContext::getIsMethodAllowed(const std::string& method) const {
	if (method == "GET") {
		return (_allowed_methods & METHOD_GET);
	} else if (method == "POST") {
		return (_allowed_methods & METHOD_POST);
	} else if (method == "DELETE") {
		return (_allowed_methods & METHOD_DELETE);
	}
	return false;
}

const std::string& LocationContext::getRoot() const {
	return this->_root;
}

const std::string& LocationContext::getAlias() const {
	return this->_alias;
}

const std::vector<std::string>& LocationContext::getIndex() const {
	return this->_index;
}

bool LocationContext::getAutoindex() const {
	return this->_autoindex;
}

int LocationContext::getRedirectCode() const {
	return this->_redirect_code;
}

const std::string& LocationContext::getRedirectUrl() const {
	return this->_redirect_url;
}

bool LocationContext::getUploadEnable() const {
	return this->_upload_enable;
}

const std::string& LocationContext::getUploadStore() const {
	return this->_upload_store;
}

const std::map<std::string, std::string>& LocationContext::getCgiInfo() const {
	return this->_cgi_info;
}
