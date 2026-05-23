#include "HttpRequest.hpp"
#include <algorithm>
#include <cctype>

namespace {
	std::string	toLower(std::string s) {
		for (std::string::iterator it = s.begin(); it != s.end(); ++it) {
			*it = static_cast<char>(std::tolower(static_cast<unsigned char>(*it)));
		}
		return (s);
	}

	std::string	trim(const std::string &s) {
		std::size_t	first;
		std::size_t	last;

		first = s.find_first_not_of(" \t");
		if (first == std::string::npos)
			return ("");
		last = s.find_last_not_of(" \t");
		return (s.substr(first, (last - first + 1)));
	}

	bool	isTokenChar(char c) {
		if (std::isalnum(static_cast<unsigned char>(c)))
			return (true);
		return (c == '!' || c == '#' || c == '$' || c == '%' || c == '&'
				|| c == '\'' || c == '*' || c == '+' || c == '-' || c == '.'
				|| c == '^' || c == '_' || c == '`' || c == '|' || c == '~');
	}

	bool	isValidMethod(const std::string &method) {
		if (method.empty())
			return (false);
		for (std::size_t i = 0; i < method.length(); ++i) {
			if (!std::isupper(static_cast<unsigned char>(method[i])))
				return (false);
		}
		return (true);
	}

	bool	isImplementedMethod(const std::string &method) {
		return (method == "GET" || method == "POST" || method == "DELETE");
	}

	bool	isSupportedTransferEncoding(const std::string &value) {
		std::string	normalized;

		normalized.reserve(value.length());
		for (std::size_t i = 0; i < value.length(); ++i) {
			if (!std::isspace(static_cast<unsigned char>(value[i]))) {
				normalized += static_cast<char>(
						std::tolower(static_cast<unsigned char>(value[i])));
			}
		}
		return (normalized == "chunked");
	}

	bool	parseDecimalSize(const std::string &value, std::size_t &result) {
		if (value.empty())
			return (false);
		result = 0;
		for (std::size_t i = 0; i < value.length(); ++i) {
			if (!std::isdigit(static_cast<unsigned char>(value[i])))
				return (false);
			result = result * 10 + static_cast<std::size_t>(value[i] - '0');
		}
		return (true);
	}

	bool	parseChunkSize(const std::string &line, std::size_t &result) {
		std::size_t	semicolon;
		std::string	sizePart;

		semicolon = line.find(';');
		sizePart = trim(line.substr(0, semicolon));
		if (sizePart.empty())
			return (false);
		result = 0;
		for (std::size_t i = 0; i < sizePart.length(); ++i) {
			char	c = sizePart[i];

			if (!std::isxdigit(static_cast<unsigned char>(c)))
				return (false);
			result *= 16;
			if (std::isdigit(static_cast<unsigned char>(c)))
				result += static_cast<std::size_t>(c - '0');
			else
				result += static_cast<std::size_t>(
						std::tolower(static_cast<unsigned char>(c)) - 'a' + 10);
		}
		return (true);
	}

	bool	hasBalancedDquotes(const std::string &value) {
		bool	escaped = false;
		bool	inQuote = false;

		for (std::size_t i = 0; i < value.length(); ++i) {
			if (escaped) {
				escaped = false;
				continue ;
			}
			if (value[i] == '\\') {
				escaped = true;
				continue ;
			}
			if (value[i] == '"')
				inQuote = !inQuote;
		}
		return (!inQuote);
	}
}

bool	HttpRequest::_parseLineState() {
	std::size_t	pos;
	std::string	line;

	pos = this->_buffer.find("\r\n");
	if (pos == std::string::npos)
		return (false);
	line = this->_buffer.substr(0, pos);
	this->_buffer.erase(0, pos + 2);
	if (this->_state == REQUEST_LINE) {
		if (line.empty())
			return (true);
		this->_parseRequestLine(line);
	} else {
		this->_parseHeader(line);
	}
	return (true);
}

bool	HttpRequest::_parseFixedBody() {
	std::size_t	remaining;
	std::size_t	toCopy;

	if (this->_contentLength > this->_maxBodySize) {
		this->_setError(413);
		return (false);
	}
	remaining = this->_contentLength - this->_body.length();
	toCopy = std::min(remaining, this->_buffer.length());
	this->_body += this->_buffer.substr(0, toCopy);
	this->_buffer.erase(0, toCopy);
	if (this->_body.length() >= this->_contentLength)
		this->_state = COMPLETE;
	return (false);
}

bool	HttpRequest::_readChunkSize() {
	std::size_t	pos;
	std::string	sizeLine;
	std::size_t	chunkSize;

	pos = this->_buffer.find("\r\n");
	if (pos == std::string::npos)
		return (false);
	sizeLine = this->_buffer.substr(0, pos);
	if (!parseChunkSize(sizeLine, chunkSize)) {
		this->_setError(400);
		return (false);
	}
	this->_chunkSize = chunkSize;
	this->_buffer.erase(0, pos + 2);
	if (this->_chunkSize == 0)
		this->_state = TRAILERS;
	return (true);
}

bool	HttpRequest::_appendChunkData() {
	if (this->_body.length() + this->_chunkSize > this->_maxBodySize) {
		this->_setError(413);
		return (false);
	}
	if (this->_buffer.length() < this->_chunkSize + 2)
		return (false);
	if (this->_buffer.substr(this->_chunkSize, 2) != "\r\n") {
		this->_setError(400);
		return (false);
	}
	this->_body += this->_buffer.substr(0, this->_chunkSize);
	this->_buffer.erase(0, this->_chunkSize + 2);
	this->_chunkSize = 0;
	return (true);
}

bool	HttpRequest::_parseChunkedBody() {
	if (this->_chunkSize == 0) {
		if (!this->_readChunkSize())
			return (false);
		if (this->_state == TRAILERS)
			return (true);
	}
	return (this->_appendChunkData());
}

bool	HttpRequest::_parseTrailer() {
	std::size_t	pos;
	std::string	line;

	pos = this->_buffer.find("\r\n");
	if (pos == std::string::npos)
		return (false);
	line = this->_buffer.substr(0, pos);
	this->_buffer.erase(0, pos + 2);
	if (line.empty()) {
		this->_state = COMPLETE;
		return (false);
	}
	return (true);
}

bool	HttpRequest::_validateRequestLineLayout(const std::string &line,
		std::size_t &firstSpace, std::size_t &secondSpace) {
	firstSpace = line.find(' ');
	if (firstSpace == std::string::npos || firstSpace == 0)
		return (false);
	secondSpace = line.find(' ', firstSpace + 1);
	if (secondSpace == std::string::npos || secondSpace == firstSpace + 1)
		return (false);
	if (line.find(' ', secondSpace + 1) != std::string::npos
		|| secondSpace == line.length() - 1)
		return (false);
	return (line.find('\t') == std::string::npos);
}

bool	HttpRequest::_validateRequestTarget() {
	if (this->_uri.empty() || this->_uri[0] != '/') {
		this->_setError(400);
		return (false);
	}
	if (this->_uri.length() > 2048) {
		this->_setError(414);
		return (false);
	}
	return (true);
}

void	HttpRequest::_splitUri() {
	std::size_t	queryPos;

	queryPos = this->_uri.find('?');
	if (queryPos != std::string::npos) {
		this->_path = this->_uri.substr(0, queryPos);
		this->_query = this->_uri.substr(queryPos + 1);
	} else {
		this->_path = this->_uri;
		this->_query = "";
	}
}

void	HttpRequest::_parseRequestLine(std::string& line) {
	std::size_t	firstSpace;
	std::size_t	secondSpace;

	if (!this->_validateRequestLineLayout(line, firstSpace, secondSpace)) {
		this->_setError(400);
		return ;
	}
	this->_method = line.substr(0, firstSpace);
	this->_uri = line.substr(firstSpace + 1, secondSpace - firstSpace - 1);
	this->_version = line.substr(secondSpace + 1);
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
	if (!this->_validateRequestTarget())
		return ;
	this->_splitUri();
	this->_normalizePath();
	this->_state = HEADERS;
}

void	HttpRequest::_finishHeaders() {
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
	} else if (this->_headers.count("content-length")) {
		this->_state = BODY;
	} else {
		this->_state = COMPLETE;
	}
}

bool	HttpRequest::_validateHeaderName(const std::string &key) {
	for (std::size_t i = 0; i < key.length(); ++i) {
		if (!isTokenChar(key[i])) {
			this->_setError(400);
			return (false);
		}
	}
	return (true);
}

bool	HttpRequest::_parseContentLengthHeader(const std::string &key,
		const std::string &value) {
	std::size_t	contentLength;

	if (key != "content-length")
		return (false);
	if (!parseDecimalSize(value, contentLength)) {
		this->_setError(400);
		return (true);
	}
	if (this->_hasContentLength && this->_contentLength != contentLength) {
		this->_setError(400);
		return (true);
	}
	this->_hasContentLength = true;
	this->_contentLength = contentLength;
	this->_headers[key] = value;
	return (true);
}

bool	HttpRequest::_validateHostHeader(const std::string &key,
		const std::string &value) {
	if (key != "host")
		return (true);
	if (this->_headers.count(key) || value.empty()
		|| value.find(',') != std::string::npos) {
		this->_setError(400);
		return (false);
	}
	return (true);
}

void	HttpRequest::_parseHeader(std::string& line) {
	std::size_t	colon;
	std::string	key;
	std::string	rawKey;
	std::string	value;

	if (line.empty()) {
		this->_finishHeaders();
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
	key = toLower(rawKey);
	value = trim(line.substr(colon + 1));
	if (!this->_validateHeaderName(key))
		return ;
	if (!hasBalancedDquotes(value)) {
		this->_setError(400);
		return ;
	}
	if (this->_parseContentLengthHeader(key, value))
		return ;
	if (!this->_validateHostHeader(key, value))
		return ;
	if (this->_headers.count(key))
		this->_headers[key] += ", " + value;
	else
		this->_headers[key] = value;
}
