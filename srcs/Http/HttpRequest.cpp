#include "HttpRequest.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <vector>

static std::string	toLower(std::string s) {
	for (std::string::iterator it = s.begin(); it != s.end(); ++it) {
		*it = static_cast<char>(std::tolower(static_cast<unsigned char>(*it)));
	}
	return (s);
}

static std::string	trim(const std::string &s) {
	size_t	first;
	size_t	last;

	first = s.find_first_not_of(" \t");
	if (first == std::string::npos)
		return ("");
	last = s.find_last_not_of(" \t");
	return (s.substr(first, (last - first + 1)));
}

static bool	isTokenChar(char c) {
	if (std::isalnum(static_cast<unsigned char>(c)))
		return (true);
	return (c == '!' || c == '#' || c == '$' || c == '%' || c == '&'
		|| c == '\'' || c == '*' || c == '+' || c == '-' || c == '.'
		|| c == '^' || c == '_' || c == '`' || c == '|' || c == '~');
}

static bool	isValidMethod(const std::string &method) {
	if (method.empty())
		return (false);
	for (size_t i = 0; i < method.length(); ++i) {
		if (!std::isupper(static_cast<unsigned char>(method[i])))
			return (false);
	}
	return (true);
}

static bool	isImplementedMethod(const std::string &method) {
	return (method == "GET" || method == "POST" || method == "DELETE");
}

static bool	isSupportedTransferEncoding(const std::string &value) {
	std::string	normalized;

	normalized.reserve(value.length());
	for (size_t i = 0; i < value.length(); ++i) {
		if (!std::isspace(static_cast<unsigned char>(value[i]))) {
			normalized += static_cast<char>(
				std::tolower(static_cast<unsigned char>(value[i])));
		}
	}
	return (normalized == "chunked");
}

static bool	parseDecimalSize(const std::string &value, size_t &result) {
	if (value.empty())
		return (false);
	result = 0;
	for (size_t i = 0; i < value.length(); ++i) {
		if (!std::isdigit(static_cast<unsigned char>(value[i])))
			return (false);
		result = result * 10 + static_cast<size_t>(value[i] - '0');
	}
	return (true);
}

static bool	parseChunkSize(const std::string &line, size_t &result) {
	size_t	semicolon;
	std::string	sizePart;

	semicolon = line.find(';');
	sizePart = trim(line.substr(0, semicolon));
	if (sizePart.empty())
		return (false);
	result = 0;
	for (size_t i = 0; i < sizePart.length(); ++i) {
		char	c;

		c = sizePart[i];
		if (!std::isxdigit(static_cast<unsigned char>(c)))
			return (false);
		result *= 16;
		if (std::isdigit(static_cast<unsigned char>(c)))
			result += static_cast<size_t>(c - '0');
		else {
			result += static_cast<size_t>(std::tolower(static_cast<unsigned char>(c)) - 'a' + 10);
		}
	}
	return (true);
}

static bool	hasBalancedDquotes(const std::string &value) {
	bool	escaped = false;
	bool	in_quote = false;

	for (size_t i = 0; i < value.length(); ++i) {
		if (escaped) {
			escaped = false;
			continue ;
		}
		if (value[i] == '\\') {
			escaped = true;
			continue ;
		}
		if (value[i] == '"')
			in_quote = !in_quote;
	}
	return (!in_quote);
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
	size_t	pos;

	if (this->_state == COMPLETE || this->_state == ERROR)
		return ;
	this->_buffer += raw_data;

	if (this->_state == REQUEST_LINE || this->_state == HEADERS) {
		if (this->_buffer.length() > 16384) {
			this->_setError(431);
			return;
		}
	}

	while (this->_state != COMPLETE && this->_state != ERROR) {
		if (this->_state == REQUEST_LINE || this->_state == HEADERS) {
			pos = this->_buffer.find("\r\n");
			if (pos == std::string::npos)
				break ;

			std::string	line = this->_buffer.substr(0, pos);
			this->_buffer.erase(0, pos + 2);

			if (this->_state == REQUEST_LINE) {
				if (line.empty())
					continue ;
				this->_parseRequestLine(line);
			}
			else
				this->_parseHeader(line);
		}
		else if (this->_state == BODY) {
			size_t	remaining;
			size_t	toCopy;

			if (this->_contentLength > this->_maxBodySize) {
				this->_setError(413);
				break ;
			}
			remaining = this->_contentLength - this->_body.length();
			toCopy = std::min(remaining, this->_buffer.length());
			this->_body += this->_buffer.substr(0, toCopy);
			this->_buffer.erase(0, toCopy);
			if (this->_body.length() >= this->_contentLength)
				this->_state = COMPLETE;
			break ;
		}
		else if (this->_state == CHUNKED_BODY) {
			if (this->_chunkSize == 0) {
				pos = this->_buffer.find("\r\n");
				if (pos == std::string::npos)
					break ;
				std::string	sizeLine = this->_buffer.substr(0, pos);
				size_t		chunkSize;

				if (!parseChunkSize(sizeLine, chunkSize)) {
					this->_setError(400);
					break ;
				}
				this->_chunkSize = chunkSize;
				this->_buffer.erase(0, pos + 2);

				if (this->_chunkSize == 0) {
					this->_state = TRAILERS;
					continue ;
				}
			}

			if (this->_body.length() + this->_chunkSize > this->_maxBodySize) {
				this->_setError(413);
				break ;
			}

			if (this->_buffer.length() >= this->_chunkSize + 2) {
				this->_body += this->_buffer.substr(0, this->_chunkSize);

				if (this->_buffer.substr(this->_chunkSize, 2) != "\r\n") {
					this->_setError(400);
					break ;
				}

				this->_buffer.erase(0, this->_chunkSize + 2);
				this->_chunkSize = 0;
			}
			else
				break ;
		}
		else if (this->_state == TRAILERS) {
			pos = this->_buffer.find("\r\n");
			if (pos == std::string::npos)
				break ;

			std::string	line = this->_buffer.substr(0, pos);
			this->_buffer.erase(0, pos + 2);

			if (line.empty()) {
				this->_state = COMPLETE;
				break ;
			}
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

size_t				HttpRequest::getContentLength() const { return (this->_contentLength); }

int					HttpRequest::getErrorCode() const { return (this->_errorCode); }

void	HttpRequest::setMaxBodySize(size_t max) {
	this->_maxBodySize = max;
	if ((this->_hasContentLength && this->_contentLength > this->_maxBodySize)
		|| this->_body.length() > this->_maxBodySize) {
		this->_setError(413);
	}
}

bool	HttpRequest::isMethodAllowed(const std::vector<std::string>& allowedMethods) const {
	for (size_t i = 0; i < allowedMethods.size(); ++i) {
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

void	HttpRequest::_parseRequestLine(std::string& line) {
	size_t	first_space;
	size_t	second_space;

	first_space = line.find(' ');
	if (first_space == std::string::npos || first_space == 0) {
		this->_setError(400);
		return ;
	}
	second_space = line.find(' ', first_space + 1);
	if (second_space == std::string::npos || second_space == first_space + 1) {
		this->_setError(400);
		return ;
	}
	if (line.find(' ', second_space + 1) != std::string::npos
		|| second_space == line.length() - 1) {
		this->_setError(400);
		return ;
	}
	if (line.find('\t') != std::string::npos) {
		this->_setError(400);
		return ;
	}
	this->_method = line.substr(0, first_space);
	this->_uri = line.substr(first_space + 1, second_space - first_space - 1);
	this->_version = line.substr(second_space + 1);
	if (!isValidMethod(this->_method)) {
		this->_setError(400);
		return ;
	}
	if (!isImplementedMethod(this->_method)) {
		this->_setError(501);
		return ;
	}
	if (this->_version != "HTTP/1.1" && this->_version != "HTTP/1.0") {
		this->_setError(505);
		return ;
	}
	if (this->_uri.empty() || this->_uri[0] != '/') {
		this->_setError(400);
		return ;
	}
	if (this->_uri.length() > 2048) {
		this->_setError(414);
		return ;
	}
	size_t	query_pos = this->_uri.find('?');
	if (query_pos != std::string::npos) {
		this->_path = this->_uri.substr(0, query_pos);
		this->_query = this->_uri.substr(query_pos + 1);
	}
	else {
		this->_path = this->_uri;
		this->_query = "";
	}
	this->_normalizePath();
	this->_state = HEADERS;
}

void	HttpRequest::_parseHeader(std::string& line) {
	size_t		colon;
	std::string	key;
	std::string	rawKey;
	std::string	value;

	if (line.empty()) {
		if (this->_version == "HTTP/1.1" && !this->_headers.count("host")) {
			this->_setError(400);
			return ;
		}
		if (this->_headers.count("transfer-encoding")) {
			if (this->_hasContentLength) {
				this->_setError(400);
				return ;
			}
			if (!isSupportedTransferEncoding(this->_headers["transfer-encoding"])) {
				this->_setError(501);
				return ;
			}
			this->_state = CHUNKED_BODY;
		}
		else if (this->_headers.count("content-length")) {
			this->_state = BODY;
		}
		else {
			this->_state = COMPLETE;
		}
		return ;
	}
	colon = line.find(':');
	if (colon == std::string::npos || colon == 0) {
		this->_setError(400);
		return ;
	}
	rawKey = line.substr(0, colon);
	if (rawKey.find_first_of(" \t") != std::string::npos) {
		this->_setError(400);
		return ;
	}
	key = ::toLower(rawKey);
	value = ::trim(line.substr(colon + 1));
	for (size_t i = 0; i < key.length(); ++i) {
		if (!isTokenChar(key[i])) {
			this->_setError(400);
			return ;
		}
	}
	if (!hasBalancedDquotes(value)) {
		this->_setError(400);
		return ;
	}
	if (key == "content-length") {
		size_t	contentLength;

		if (!parseDecimalSize(value, contentLength)) {
			this->_setError(400);
			return ;
		}
		if (this->_hasContentLength && this->_contentLength != contentLength) {
			this->_setError(400);
			return ;
		}
		this->_hasContentLength = true;
		this->_contentLength = contentLength;
		this->_headers[key] = value;
		return ;
	}
	if (key == "host") {
		if (this->_headers.count(key) || value.empty()
			|| value.find(',') != std::string::npos) {
			this->_setError(400);
			return ;
		}
	}
	if (this->_headers.count(key)) {
		this->_headers[key] += ", " + value;
	}
	else {
		this->_headers[key] = value;
	}
}

void	HttpRequest::_normalizePath() {
	std::vector<std::string>	segments;
	std::stringstream			ss(this->_path);
	std::string					segment;
	std::string					res = "";

	while (std::getline(ss, segment, '/')) {
		if (segment == "" || segment == ".") continue;
		if (segment == "..") { if (!segments.empty()) segments.pop_back(); }
		else segments.push_back(segment);
	}
	for (size_t i = 0; i < segments.size(); ++i) res += "/" + segments[i];
	if (res == "") res = "/";
	if (this->_path.length() > 1 && this->_path[this->_path.length() - 1] == '/') res += "/";
	this->_path = res;
}
