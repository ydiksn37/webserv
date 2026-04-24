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
		_upload_store(""),
		_client_max_body_size(1048576) {}

LocationContext::~LocationContext() {}

LocationContext::LocationContext(const LocationContext& other)
	: _path(other._path),
		_allowed_methods(other._allowed_methods),
		_root(other._root),
		_alias(other._alias),
		_index(other._index),
		_autoindex(other._autoindex),
		_redirect_code(other._redirect_code),
		_redirect_url(other._redirect_url),
		_upload_enable(other._upload_enable),
		_upload_store(other._upload_store),
		_cgi_extensions(other._cgi_extensions),
		_cgi_info(other._cgi_info),
		_client_max_body_size(other._client_max_body_size),
		_error_pages(other._error_pages) {}

LocationContext& LocationContext::operator=(const LocationContext& other) {
	if (this != &other) {
		this->_path = other._path;
		this->_allowed_methods = other._allowed_methods;
		this->_root = other._root;
		this->_alias = other._alias;
		this->_index = other._index;
		this->_autoindex = other._autoindex;
		this->_redirect_code = other._redirect_code;
		this->_redirect_url = other._redirect_url;
		this->_upload_enable = other._upload_enable;
		this->_upload_store = other._upload_store;
		this->_cgi_extensions = other._cgi_extensions;
		this->_cgi_info = other._cgi_info;
		this->_client_max_body_size = other._client_max_body_size;
		this->_error_pages = other._error_pages;
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
		_upload_store(""),
		_client_max_body_size(1048576) {}

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

void LocationContext::setCgiExtension(const std::string& ext) {
	this->_cgi_extensions.push_back(ext);
}

void LocationContext::setCgiPath(const std::string& ext, const std::string& path) {
	this->_cgi_info[ext] = path;
}

void LocationContext::setClientMaxBodySize(size_t size) {
	this->_client_max_body_size = size;
}

void LocationContext::setErrorPage(int status_code, const std::string& path) {
	this->_error_pages[status_code] = path;
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

const std::vector<std::string>& LocationContext::getCgiExtensions() const {
	return this->_cgi_extensions;
}

const std::map<std::string, std::string>& LocationContext::getCgiInfo() const {
	return this->_cgi_info;
}

size_t LocationContext::getClientMaxBodySize() const {
	return this->_client_max_body_size;
}

const std::map<int, std::string>& LocationContext::getErrorPages() const {
	return this->_error_pages;
}

void LocationContext::validate() const {
	if (this->_redirect_code != 0 && (!this->_root.empty() || !this->_alias.empty())) {
		throw std::runtime_error("Conflict: 'return' directive cannot be used with 'root' or 'alias' in location " + this->_path);
	}
	if (this->_autoindex && !this->_cgi_info.empty()) {
		throw std::runtime_error("Conflict: 'autoindex' and 'cgi' cannot both be active in location " + this->_path);
	}
}
