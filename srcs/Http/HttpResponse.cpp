#include "HttpResponse.hpp"
#include <sstream>
#include <ctime>
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>

namespace {
	void	addInformationalStatuses(std::map<int, std::string> &m) {
		m[100] = "Continue";
		m[101] = "Switching Protocols";
		m[102] = "Processing";
		m[103] = "Early Hints";
	}

	void	addSuccessStatuses(std::map<int, std::string> &m) {
		m[200] = "OK";
		m[201] = "Created";
		m[202] = "Accepted";
		m[203] = "Non-Authoritative Information";
		m[204] = "No Content";
		m[205] = "Reset Content";
		m[206] = "Partial Content";
	}

	void	addRedirectionStatuses(std::map<int, std::string> &m) {
		m[300] = "Multiple Choices";
		m[301] = "Moved Permanently";
		m[302] = "Found";
		m[303] = "See Other";
		m[304] = "Not Modified";
		m[305] = "Use Proxy";
		m[307] = "Temporary Redirect";
		m[308] = "Permanent Redirect";
	}

	void	addClientErrorStatuses(std::map<int, std::string> &m) {
		m[400] = "Bad Request";
		m[401] = "Unauthorized";
		m[402] = "Payment Required";
		m[403] = "Forbidden";
		m[404] = "Not Found";
		m[405] = "Method Not Allowed";
		m[406] = "Not Acceptable";
		m[407] = "Proxy Authentication Required";
		m[408] = "Request Timeout";
		m[409] = "Conflict";
		m[410] = "Gone";
		m[411] = "Length Required";
		m[412] = "Precondition Failed";
		m[413] = "Payload Too Large";
		m[414] = "URI Too Long";
		m[415] = "Unsupported Media Type";
		m[416] = "Range Not Satisfiable";
		m[417] = "Expectation Failed";
		m[421] = "Misdirected Request";
		m[422] = "Unprocessable Entity";
		m[425] = "Too Early";
		m[426] = "Upgrade Required";
		m[429] = "Too Many Requests";
		m[431] = "Request Header Fields Too Large";
	}

	void	addServerErrorStatuses(std::map<int, std::string> &m) {
		m[500] = "Internal Server Error";
		m[501] = "Not Implemented";
		m[502] = "Bad Gateway";
		m[503] = "Service Unavailable";
		m[504] = "Gateway Timeout";
		m[505] = "HTTP Version Not Supported";
		m[506] = "Variant Also Negotiates";
		m[507] = "Insufficient Storage";
		m[508] = "Loop Detected";
		m[510] = "Not Extended";
		m[511] = "Network Authentication Required";
	}

	std::string	htmlEscape(const std::string &value) {
		std::string	escaped;

		for (std::size_t i = 0; i < value.length(); ++i) {
			if (value[i] == '&')
				escaped += "&amp;";
			else if (value[i] == '<')
				escaped += "&lt;";
			else if (value[i] == '>')
				escaped += "&gt;";
			else if (value[i] == '"')
				escaped += "&quot;";
			else
				escaped += value[i];
		}
		return (escaped);
	}

	std::string	normalizeBaseUri(const std::string &uri) {
		if (uri.empty())
			return ("/");
		return (uri);
	}

	void	writeDirectoryListingHeader(std::stringstream &ss,
			const std::string &baseUri) {
		ss << "<html>\r\n<head><title>Index of " << htmlEscape(baseUri)
			<< "</title></head>\r\n";
		ss << "<body>\r\n<h1>Index of " << htmlEscape(baseUri)
			<< "</h1><hr><pre>\r\n";
	}

	void	writeDirectoryEntry(std::stringstream &ss,
			const std::string &baseUri, const std::string &name) {
		std::string	displayName;

		displayName = htmlEscape(name);
		ss << "<a href=\"" << htmlEscape(baseUri);
		if (baseUri[baseUri.length() - 1] != '/')
			ss << "/";
		ss << displayName << "\">" << displayName << "</a>\r\n";
	}
}

HttpResponse::HttpResponse() : _version("HTTP/1.1"), _statusCode(200), _reasonPhrase("OK") {
	this->setHeader("Date", this->_getCurrentDate());
	this->setHeader("Server", "Webserv/1.0");
}

HttpResponse::HttpResponse(const HttpResponse& other)
	: _version(other._version), _statusCode(other._statusCode),
		_reasonPhrase(other._reasonPhrase), _headers(other._headers),
		_body(other._body) {}

HttpResponse&	HttpResponse::operator=(const HttpResponse& other) {
	if (this != &other) {
		this->_version = other._version;
		this->_statusCode = other._statusCode;
		this->_reasonPhrase = other._reasonPhrase;
		this->_headers = other._headers;
		this->_body = other._body;
	}
	return (*this);
}

HttpResponse::~HttpResponse() {}

std::map<int, std::string>		HttpResponse::_initStatusMessages() {
	std::map<int, std::string>	m;

	addInformationalStatuses(m);
	addSuccessStatuses(m);
	addRedirectionStatuses(m);
	addClientErrorStatuses(m);
	addServerErrorStatuses(m);
	return (m);
}

void	HttpResponse::setStatusCode(int code) {
	this->_statusCode = code;
	this->_reasonPhrase = this->_getStatusMessage(code);
	if (code >= 400) {
		this->setBody(this->_generateDefaultErrorPage(code));
		this->setHeader("Content-Type", "text/html");
	}
}

void	HttpResponse::setReasonPhrase(const std::string& phrase) {
	this->_reasonPhrase = phrase;
}

void	HttpResponse::setHeader(const std::string& key, const std::string& value) {
	if (value.find("\r\n") != std::string::npos || key.find("\r\n") != std::string::npos)
		return;
	this->_headers[this->_normalizeHeaderKey(key)] = value;
}

void	HttpResponse::setBody(const std::string& body) {
	std::stringstream	ss;

	this->_body = body;
	ss << body.length();
	this->setHeader("Content-Length", ss.str());
}

void	HttpResponse::setBodyFromFile(const std::string& filepath) {
	struct stat				fileStat;
	std::ifstream			file(filepath.c_str(), std::ios::binary);
	std::stringstream	buffer;

	if (stat(filepath.c_str(), &fileStat) != 0) {
		this->setStatusCode(404);
		return ;
	}
	if (S_ISDIR(fileStat.st_mode)) {
		this->setStatusCode(403);
		return ;
	}
	if (!file.is_open()) {
		this->setStatusCode(403);
		return ;
	}
	buffer << file.rdbuf();
	this->setBody(buffer.str());
	this->setHeader("Content-Type", this->getContentType(filepath));
}

void	HttpResponse::setHttpVersion(const std::string& version) {
	this->_version = version;
}

void	HttpResponse::setRedirect(int code, const std::string& location) {
	this->setStatusCode(code);
	this->setHeader("Location", location);
	this->setBody("");
}

const std::map<std::string, std::string>&	HttpResponse::getHeaders() const {
	return (this->_headers);
}

int	HttpResponse::getStatusCode() const {
	return (this->_statusCode);
}

std::string	HttpResponse::serialize() const {
	std::stringstream	ss;

	ss << this->_version << " " << this->_statusCode << " " << this->_reasonPhrase << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it = this->_headers.begin(); it != this->_headers.end(); ++it) {
		ss << it->first << ": " << it->second << "\r\n";
	}
	ss << "\r\n";
	ss << this->_body;
	return (ss.str());
}

std::string	HttpResponse::_getStatusMessage(int code) const {
	static std::map<int, std::string>						m = _initStatusMessages();
	std::map<int, std::string>::const_iterator	it;

	it = m.find(code);
	if (it != m.end())
		return (it->second);
	return ("Unknown");
}

std::string	HttpResponse::_getCurrentDate() const {
	std::time_t	now;
	std::tm			*tm;
	char				buf[100];

	now = std::time(0);
	tm = std::gmtime(&now);
	std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", tm);
	return (std::string(buf));
}

std::string	HttpResponse::_normalizeHeaderKey(const std::string& key) const {
	std::string	normalized;
	bool				capitalize_next;

	normalized = key;
	capitalize_next = true;
	for (std::size_t i = 0; i < normalized.length(); ++i) {
		if (capitalize_next) {
			normalized[i] = static_cast<char>(
				std::toupper(static_cast<unsigned char>(normalized[i])));
			capitalize_next = false;
		} else {
			normalized[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(normalized[i])));
		}
		if (normalized[i] == '-')
			capitalize_next = true;
	}
	return (normalized);
}

std::string	HttpResponse::_generateDefaultErrorPage(int code) const
{
	std::stringstream	ss;
	std::string				message;

	message = this->_getStatusMessage(code);
	ss << "<html>\r\n";
	ss << "<head><title>" << code << " " << message << "</title></head>\r\n";
	ss << "<body>\r\n";
	ss << "<center><h1>" << code << " " << message << "</h1></center>\r\n";
	ss << "<hr><center>Webserv/1.0</center>\r\n";
	ss << "</body>\r\n";
	ss << "</html>\r\n";
	return (ss.str());
}

std::string	HttpResponse::generateDirectoryListing(const std::string& path, const std::string& uri)
{
	std::stringstream	ss;
	DIR								*dir;
	struct dirent			*entry;
	std::string				baseUri;

	dir = opendir(path.c_str());
	if (dir == NULL)
		return ("");
	baseUri = normalizeBaseUri(uri);
	writeDirectoryListingHeader(ss, baseUri);

	while ((entry = readdir(dir)) != NULL) {
		std::string	name = entry->d_name;

		if (name == ".")
			continue;
		writeDirectoryEntry(ss, baseUri, name);
	}
	closedir(dir);

	ss << "</pre><hr></body>\r\n</html>\r\n";
	return (ss.str());
}

std::string	HttpResponse::getContentType(const std::string& path)
{
	std::size_t	dot_pos;

	dot_pos = path.find_last_of('.');
	if (dot_pos == std::string::npos)
		return ("application/octet-stream");

	std::string	ext = path.substr(dot_pos);
	if (ext == ".html" || ext == ".htm") return ("text/html");
	if (ext == ".css") return ("text/css");
	if (ext == ".js") return ("application/javascript");
	if (ext == ".png") return ("image/png");
	if (ext == ".jpg" || ext == ".jpeg") return ("image/jpeg");
	if (ext == ".gif") return ("image/gif");
	if (ext == ".txt") return ("text/plain");
	if (ext == ".pdf") return ("application/pdf");
	if (ext == ".ico") return ("image/x-icon");
	if (ext == ".json") return ("application/json");

	return ("application/octet-stream");
}
