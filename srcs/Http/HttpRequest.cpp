#include "HttpRequest.hpp"
#include <cctype>
#include <sstream>
#include <vector>

namespace {
	std::string	toLower(std::string s) {
		for (std::string::iterator it = s.begin(); it != s.end(); ++it) {
			*it = static_cast<char>(std::tolower(static_cast<unsigned char>(*it)));
		}
		return (s);
	}
}

HttpRequest::HttpRequest()
	: _state(REQUEST_LINE), _routingResolved(false), _method(""), _uri(""), _path(""), _query(""),
		_version(""), _headers(), _body(""), _buffer(""), _empty(""), _chunkSize(0),
		_contentLength(0), _hasContentLength(false), _errorCode(0),
		_maxBodySize(1048576) {}

HttpRequest::HttpRequest(const HttpRequest &other)
	: _state(other._state), _routingResolved(other._routingResolved), _method(other._method), _uri(other._uri),
		_path(other._path), _query(other._query), _version(other._version),
		_headers(other._headers), _body(other._body), _buffer(other._buffer),
		_empty(other._empty), _chunkSize(other._chunkSize),
		_contentLength(other._contentLength),
		_hasContentLength(other._hasContentLength),
		_errorCode(other._errorCode), _maxBodySize(other._maxBodySize) {}

HttpRequest&	HttpRequest::operator=(const HttpRequest &other) {
	if (this != &other) {
		this->_state = other._state;
		this->_routingResolved = other._routingResolved;
		this->_method = other._method;
		this->_uri = other._uri;
		this->_path = other._path;
		this->_query = other._query;
		this->_version = other._version;
		this->_headers = other._headers;
		this->_body = other._body;
		this->_buffer = other._buffer;
		this->_empty = other._empty;
		this->_chunkSize = other._chunkSize;
		this->_contentLength = other._contentLength;
		this->_hasContentLength = other._hasContentLength;
		this->_errorCode = other._errorCode;
		this->_maxBodySize = other._maxBodySize;
	}
	return (*this);
}

HttpRequest::~HttpRequest() {}

void	HttpRequest::_setError(int errorCode) {
	this->_state = ERROR;
	this->_errorCode = errorCode;
	this->_buffer.clear();
}

void	HttpRequest::parse(const std::string &raw_data) {
	if (this->_state == COMPLETE || this->_state == ERROR)
		return ;
	this->_buffer += raw_data;
	if (this->_state == REQUEST_LINE || this->_state == HEADERS) {
		if (this->_buffer.length() > (1 << 14)) {
			this->_setError(431);
			return ;
		}
	}
	while (this->_state != COMPLETE && this->_state != ERROR) {
		if (this->_state == REQUEST_LINE || this->_state == HEADERS) {
			if (!this->_parseLineState())
				break ;
		} else if (this->_state == BODY) {
			if (!this->_parseFixedBody())
				break ;
		} else if (this->_state == CHUNKED_BODY) {
			if (!this->_parseChunkedBody())
				break ;
		} else if (this->_state == TRAILERS) {
			if (!this->_parseTrailer())
				break ;
		}
	}
}

bool	HttpRequest::isCompleted() const { return (this->_state == COMPLETE); }

bool	HttpRequest::isError() const { return (this->_state == ERROR); }

bool	HttpRequest::isHeaderFinished() const { return (this->_state == BODY || this->_state == CHUNKED_BODY || this->_state == TRAILERS || this->_state == COMPLETE || this->_state == ERROR); }

bool	HttpRequest::isRoutingResolved() const { return (this->_routingResolved); }

void	HttpRequest::setRoutingResolved(bool resolved) { this->_routingResolved = resolved; }

void	HttpRequest::clear() {
	this->_state = REQUEST_LINE;
	this->_routingResolved = false;
	this->_method.clear();
	this->_uri.clear();
	this->_path.clear();
	this->_query.clear();
	this->_version.clear();
	this->_headers.clear();
	this->_body.clear();
	this->_chunkSize = 0;
	this->_contentLength = 0;
	this->_hasContentLength = false;
	this->_errorCode = 0;
}

const std::string&	HttpRequest::getMethod() const { return (this->_method); }

const std::string&	HttpRequest::getUri() const { return (this->_uri); }

const std::string&	HttpRequest::getPath() const { return (this->_path); }

const std::string&	HttpRequest::getQuery() const { return (this->_query); }

const std::string&	HttpRequest::getVersion() const { return (this->_version); }

const std::string&	HttpRequest::getBody() const { return (this->_body); }

std::size_t				HttpRequest::getContentLength() const { return (this->_contentLength); }

int					HttpRequest::getErrorCode() const { return (this->_errorCode); }

void	HttpRequest::setMaxBodySize(std::size_t max) {
	this->_maxBodySize = max;
	if ((this->_hasContentLength && this->_contentLength > this->_maxBodySize)
		|| this->_body.length() > this->_maxBodySize) {
		this->_setError(413);
	}
}

bool	HttpRequest::isMethodAllowed(const std::vector<std::string>& allowedMethods) const {
	for (std::size_t i = 0; i < allowedMethods.size(); ++i) {
		if (this->_method == allowedMethods[i])
			return (true);
	}
	return (false);
}

const std::string&	HttpRequest::getHeader(const std::string& key) const {
	std::map<std::string, std::string>::const_iterator	it;

	it = this->_headers.find(::toLower(key));
	if (it != this->_headers.end())
		return (it->second);
	return (this->_empty);
}

const std::map<std::string, std::string>&	HttpRequest::getHeaders() const {
	return (this->_headers);
}

void	HttpRequest::_normalizePath() {
	std::vector<std::string>	segments;
	std::stringstream					ss(this->_path);
	std::string								segment;
	std::string								res = "";

	while (std::getline(ss, segment, '/')) {
		if (segment == "" || segment == ".") continue;
		if (segment == "..") { if (!segments.empty()) segments.pop_back(); }
		else segments.push_back(segment);
	}
	for (std::size_t i = 0; i < segments.size(); ++i) res += "/" + segments[i];
	if (res == "") res = "/";
	if (this->_path.length() > 1 && this->_path[this->_path.length() - 1] == '/') res += "/";
	this->_path = res;
}
