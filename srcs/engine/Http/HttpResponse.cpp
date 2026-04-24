#include "engine/Http/HttpResponse.hpp"
#include <sstream>
#include <ctime>
#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>

// デフォルトコンストラクタ：初期値を設定し、必須ヘッダーを自動付与する
HttpResponse::HttpResponse()
	: _version("HTTP/1.1"), _statusCode(200), _reasonPhrase("OK")
{
	// インスタンス生成時の時刻をDateヘッダーに設定
	this->setHeader("Date", this->_getCurrentDate());
	// サーバー名をServerヘッダーに設定
	this->setHeader("Server", "Webserv/1.0");
}

// コピーコンストラクタ
HttpResponse::HttpResponse(const HttpResponse& other)
	: _version(other._version), _statusCode(other._statusCode),
	  _reasonPhrase(other._reasonPhrase), _headers(other._headers),
	  _body(other._body)
{
}

// 代入演算子
HttpResponse&	HttpResponse::operator=(const HttpResponse& other)
{
	if (this != &other)
	{
		this->_version = other._version;
		this->_statusCode = other._statusCode;
		this->_reasonPhrase = other._reasonPhrase;
		this->_headers = other._headers;
		this->_body = other._body;
	}
	return (*this);
}

// デストラクタ
HttpResponse::~HttpResponse()
{
}

// HTTPステータスコードとメッセージの対応表
std::map<int, std::string>	HttpResponse::_initStatusMessages()
{
	std::map<int, std::string>	m;

	m[100] = "Continue";
	m[101] = "Switching Protocols";
	m[200] = "OK";
	m[201] = "Created";
	m[202] = "Accepted";
	m[203] = "Non-Authoritative Information";
	m[204] = "No Content";
	m[205] = "Reset Content";
	m[206] = "Partial Content";
	m[300] = "Multiple Choices";
	m[301] = "Moved Permanently";
	m[302] = "Found";
	m[303] = "See Other";
	m[304] = "Not Modified";
	m[305] = "Use Proxy";
	m[307] = "Temporary Redirect";
	m[308] = "Permanent Redirect";
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
	m[426] = "Upgrade Required";
	m[500] = "Internal Server Error";
	m[501] = "Not Implemented";
	m[502] = "Bad Gateway";
	m[503] = "Service Unavailable";
	m[504] = "Gateway Timeout";
	m[505] = "HTTP Version Not Supported";
	return (m);
}

// ステータスコード設定
void	HttpResponse::setStatusCode(int code)
{
	this->_statusCode = code;
	this->_reasonPhrase = this->_getStatusMessage(code);
	// エラー時はデフォルトエラーページを生成
	if (code >= 400)
	{
		this->setBody(this->_generateDefaultErrorPage(code));
		this->setHeader("Content-Type", "text/html");
	}
}

// リーズンフレーズ設定
void	HttpResponse::setReasonPhrase(const std::string& phrase)
{
	this->_reasonPhrase = phrase;
}

// ヘッダー設定（インジェクション対策済み）
void	HttpResponse::setHeader(const std::string& key, const std::string& value)
{
	if (value.find("\r\n") != std::string::npos || key.find("\r\n") != std::string::npos)
	{
		return ;
	}
	this->_headers[this->_normalizeHeaderKey(key)] = value;
}

// ボディ設定
void	HttpResponse::setBody(const std::string& body)
{
	std::stringstream	ss;

	this->_body = body;
	ss << body.length();
	this->setHeader("Content-Length", ss.str());
}

// ファイルからボディを読み込む
void	HttpResponse::setBodyFromFile(const std::string& filepath)
{
	std::ifstream		file(filepath.c_str(), std::ios::binary);
	std::stringstream	buffer;

	// ファイルが開けなかった場合は404エラーを設定して終了
	if (!file.is_open())
	{
		this->setStatusCode(404);
		return ;
	}
	// ファイル内容を一括読み込み
	buffer << file.rdbuf();
	this->setBody(buffer.str());
	// 拡張子からContent-Typeを自動設定
	this->setHeader("Content-Type", this->getContentType(filepath));
}

// HTTPバージョン設定
void	HttpResponse::setHttpVersion(const std::string& version)
{
	this->_version = version;
}

// リダイレクト設定
void	HttpResponse::setRedirect(int code, const std::string& location)
{
	this->setStatusCode(code);
	this->setHeader("Location", location);
	this->setBody("");
}

// ヘッダーマップの取得
const std::map<std::string, std::string>&	HttpResponse::getHeaders() const
{
	return (this->_headers);
}

// シリアライズ
std::string	HttpResponse::serialize() const
{
	std::stringstream	ss;

	ss << this->_version << " " << this->_statusCode << " " << this->_reasonPhrase << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it = this->_headers.begin(); it != this->_headers.end(); ++it)
	{
		ss << it->first << ": " << it->second << "\r\n";
	}
	ss << "\r\n";
	ss << this->_body;
	return (ss.str());
}

// ステータスメッセージ取得
std::string	HttpResponse::_getStatusMessage(int code) const
{
	static std::map<int, std::string>			m = _initStatusMessages();
	std::map<int, std::string>::const_iterator	it;

	it = m.find(code);
	if (it != m.end())
	{
		return (it->second);
	}
	return ("Unknown");
}

// 現在時刻取得
std::string	HttpResponse::_getCurrentDate() const
{
	time_t		now;
	struct tm	*tm;
	char		buf[100];

	now = time(0);
	tm = gmtime(&now);
	strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", tm);
	return (std::string(buf));
}

// ヘッダー名正規化
std::string	HttpResponse::_normalizeHeaderKey(const std::string& key) const
{
	std::string	normalized;
	bool		capitalize_next;

	normalized = key;
	capitalize_next = true;
	for (size_t i = 0; i < normalized.length(); ++i)
	{
		if (capitalize_next && std::tolower(normalized[i]))
		{
			normalized[i] = static_cast<char>(std::toupper(normalized[i]));
			capitalize_next = false;
		}
		else if (!capitalize_next && std::toupper(normalized[i]))
		{
			normalized[i] = static_cast<char>(std::tolower(normalized[i]));
		}
		if (normalized[i] == '-')
		{
			capitalize_next = true;
		}
	}
	return (normalized);
}

// デフォルトエラーページ生成
std::string	HttpResponse::_generateDefaultErrorPage(int code) const
{
	std::stringstream	ss;
	std::string			message;

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

// ディレクトリリスティング生成
std::string	HttpResponse::generateDirectoryListing(const std::string& path, const std::string& uri)
{
	std::stringstream	ss;
	DIR					*dir;
	struct dirent		*entry;

	dir = opendir(path.c_str());
	if (dir == NULL)
	{
		return ("");
	}

	ss << "<html>\r\n<head><title>Index of " << uri << "</title></head>\r\n";
	ss << "<body>\r\n<h1>Index of " << uri << "</h1><hr><pre>\r\n";

	while ((entry = readdir(dir)) != NULL)
	{
		std::string	name = entry->d_name;
		if (name == ".")
		{
			continue;
		}
		ss << "<a href=\"" << uri;
		if (uri[uri.length() - 1] != '/')
		{
			ss << "/";
		}
		ss << name << "\">" << name << "</a>\r\n";
	}
	closedir(dir);

	ss << "</pre><hr></body>\r\n</html>\r\n";
	return (ss.str());
}

// MIMEタイプ取得
std::string	HttpResponse::getContentType(const std::string& path)
{
	size_t	dot_pos;

	dot_pos = path.find_last_of('.');
	if (dot_pos == std::string::npos)
	{
		return ("application/octet-stream");
	}

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
