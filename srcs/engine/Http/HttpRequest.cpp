#include "engine/Http/HttpRequest.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <vector>

// 小文字変換ヘルパー
static std::string	toLower(std::string s)
{
	for (std::string::iterator it = s.begin(); it != s.end(); ++it)
	{
		*it = static_cast<char>(std::tolower(static_cast<unsigned char>(*it)));
	}
	return (s);
}

// 空白除去ヘルパー
static std::string	trim(const std::string &s)
{
	size_t	first;
	size_t	last;

	first = s.find_first_not_of(" \t");
	if (first == std::string::npos)
	{
		return ("");
	}
	last = s.find_last_not_of(" \t");
	return (s.substr(first, (last - first + 1)));
}

// コンストラクタ：初期値（デフォルトサイズは1MBなど）
HttpRequest::HttpRequest()
	: _state(REQUEST_LINE), _method(""), _uri(""), _path(""), _query(""),
	  _version(""), _headers(), _body(""), _buffer(""), _empty(""), _chunkSize(0),
	  _errorCode(0), _maxBodySize(1048576) // デフォルト 1MB
{
}

HttpRequest::HttpRequest(const HttpRequest &other)
	: _state(other._state), _method(other._method), _uri(other._uri),
	  _path(other._path), _query(other._query), _version(other._version),
	  _headers(other._headers), _body(other._body), _buffer(other._buffer),
	  _empty(other._empty), _chunkSize(other._chunkSize),
	  _errorCode(other._errorCode), _maxBodySize(other._maxBodySize)
{
}

HttpRequest&	HttpRequest::operator=(const HttpRequest &other)
{
	if (this != &other)
	{
		this->_state = other._state;
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
		this->_errorCode = other._errorCode;
		this->_maxBodySize = other._maxBodySize;
	}
	return (*this);
}

HttpRequest::~HttpRequest()
{
}

// 解析メインルーチン
void	HttpRequest::parse(const std::string &raw_data)
{
	size_t	pos;

	this->_buffer += raw_data;
	while (this->_state != COMPLETE && this->_state != ERROR)
	{
		if (this->_state == REQUEST_LINE || this->_state == HEADERS)
		{
			pos = this->_buffer.find("\r\n");
			if (pos == std::string::npos)
			{
				break ;
			}

			std::string	line = this->_buffer.substr(0, pos);
			this->_buffer.erase(0, pos + 2);

			if (this->_state == REQUEST_LINE)
			{
				this->_parseRequestLine(line);
			}
			else
			{
				this->_parseHeader(line);
			}
		}
		else if (this->_state == BODY)
		{
			if (this->_headers.count("content-length"))
			{
				size_t	len = static_cast<size_t>(std::strtol(this->_headers["content-length"].c_str(), NULL, 10));
				
				if (len > this->_maxBodySize)
				{
					this->_state = ERROR;
					this->_errorCode = 413;
					break ;
				}

				size_t	remaining = len - this->_body.length();
				size_t	to_copy = std::min(remaining, this->_buffer.length());

				this->_body += this->_buffer.substr(0, to_copy);
				this->_buffer.erase(0, to_copy);

				if (this->_body.length() >= len)
				{
					this->_state = COMPLETE;
				}
			}
			else
			{
				this->_state = COMPLETE;
			}
			break ;
		}
		else if (this->_state == CHUNKED_BODY)
		{
			if (this->_chunkSize == 0)
			{
				pos = this->_buffer.find("\r\n");
				if (pos == std::string::npos)
				{
					break ;
				}
				
				std::string	size_line = this->_buffer.substr(0, pos);
				char		*endptr;
				long		size = std::strtol(size_line.c_str(), &endptr, 16);

				if (*endptr != '\0' && *endptr != ';')
				{
					this->_state = ERROR;
					this->_errorCode = 400;
					break ;
				}

				this->_chunkSize = static_cast<size_t>(size);
				this->_buffer.erase(0, pos + 2);

				if (this->_chunkSize == 0)
				{
					this->_state = TRAILERS;
					continue ;
				}
			}

			if (this->_body.length() + this->_chunkSize > this->_maxBodySize)
			{
				this->_state = ERROR;
				this->_errorCode = 413;
				break ;
			}

			if (this->_buffer.length() >= this->_chunkSize + 2)
			{
				this->_body += this->_buffer.substr(0, this->_chunkSize);

				if (this->_buffer.substr(this->_chunkSize, 2) != "\r\n")
				{
					this->_state = ERROR;
					this->_errorCode = 400;
					break ;
				}

				this->_buffer.erase(0, this->_chunkSize + 2);
				this->_chunkSize = 0;
			}
			else
			{
				break ;
			}
		}
		else if (this->_state == TRAILERS)
		{
			pos = this->_buffer.find("\r\n");
			if (pos == std::string::npos)
			{
				break ;
			}
			
			std::string	line = this->_buffer.substr(0, pos);
			this->_buffer.erase(0, pos + 2);

			if (line.empty())
			{
				this->_state = COMPLETE;
				break ;
			}
		}
	}
}

bool	HttpRequest::isCompleted() const { return (this->_state == COMPLETE); }
bool	HttpRequest::isError() const { return (this->_state == ERROR); }

void	HttpRequest::clear()
{
	this->_state = REQUEST_LINE;
	this->_method.clear();
	this->_uri.clear();
	this->_path.clear();
	this->_query.clear();
	this->_version.clear();
	this->_headers.clear();
	this->_body.clear();
	this->_buffer.clear();
	this->_chunkSize = 0;
	this->_errorCode = 0;
}

const std::string&	HttpRequest::getMethod() const { return (this->_method); }
const std::string&	HttpRequest::getUri() const { return (this->_uri); }
const std::string&	HttpRequest::getPath() const { return (this->_path); }
const std::string&	HttpRequest::getQuery() const { return (this->_query); }
const std::string&	HttpRequest::getVersion() const { return (this->_version); }
const std::string&	HttpRequest::getBody() const { return (this->_body); }
int					HttpRequest::getErrorCode() const { return (this->_errorCode); }

void	HttpRequest::setMaxBodySize(size_t max) { this->_maxBodySize = max; }

// 指定されたメソッドが許可されているかチェック
bool	HttpRequest::isMethodAllowed(const std::vector<std::string>& allowedMethods) const
{
	for (size_t i = 0; i < allowedMethods.size(); ++i)
	{
		if (this->_method == allowedMethods[i])
		{
			return (true);
		}
	}
	return (false);
}

const std::string&	HttpRequest::getHeader(const std::string& key) const
{
	std::map<std::string, std::string>::const_iterator	it;

	it = this->_headers.find(::toLower(key));
	if (it != this->_headers.end())
	{
		return (it->second);
	}
	return (this->_empty);
}

const std::map<std::string, std::string>&	HttpRequest::getHeaders() const
{
	return (this->_headers);
}

void	HttpRequest::_parseRequestLine(std::string& line)
{
	if (line.empty()) return ;
	std::stringstream	ss(line);
	if (!(ss >> this->_method >> this->_uri >> this->_version))
	{
		this->_state = ERROR;
		this->_errorCode = 400;
		return ;
	}
	if (this->_uri.length() > 2048)
	{
		this->_state = ERROR;
		this->_errorCode = 414;
		return ;
	}
	size_t	query_pos = this->_uri.find('?');
	if (query_pos != std::string::npos)
	{
		this->_path = this->_uri.substr(0, query_pos);
		this->_query = this->_uri.substr(query_pos + 1);
	}
	else
	{
		this->_path = this->_uri;
		this->_query = "";
	}
	this->_normalizePath();
	this->_state = HEADERS;
}

void	HttpRequest::_parseHeader(std::string& line)
{
	if (line.empty())
	{
		if (this->_version == "HTTP/1.1" && !this->_headers.count("host"))
		{
			this->_state = ERROR;
			this->_errorCode = 400;
			return ;
		}
		if (this->_headers.count("transfer-encoding") && this->_headers["transfer-encoding"] == "chunked")
			this->_state = CHUNKED_BODY;
		else if (this->_headers.count("content-length"))
			this->_state = BODY;
		else
			this->_state = COMPLETE;
		return ;
	}
	size_t	colon = line.find(':');
	if (colon != std::string::npos)
	{
		std::string	key = ::toLower(::trim(line.substr(0, colon)));
		std::string	value = ::trim(line.substr(colon + 1));
		if (this->_headers.count(key)) this->_headers[key] += ", " + value;
		else this->_headers[key] = value;
	}
}

void	HttpRequest::_normalizePath()
{
	std::vector<std::string>	segments;
	std::stringstream			ss(this->_path);
	std::string					segment;
	std::string					res = "";

	while (std::getline(ss, segment, '/'))
	{
		if (segment == "" || segment == ".") continue;
		if (segment == "..") { if (!segments.empty()) segments.pop_back(); }
		else segments.push_back(segment);
	}
	for (size_t i = 0; i < segments.size(); ++i) res += "/" + segments[i];
	if (res == "") res = "/";
	if (this->_path.length() > 1 && this->_path[this->_path.length() - 1] == '/') res += "/";
	this->_path = res;
}
