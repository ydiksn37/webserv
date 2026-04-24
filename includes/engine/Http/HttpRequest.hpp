#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <cstddef>
# include <string>
# include <map>
# include <vector>

// HTTPリクエストを解析・保持するクラス
class HttpRequest
{
public:
	// 正統なクラス設計
	HttpRequest();
	HttpRequest(const HttpRequest &other);
	HttpRequest&	operator=(const HttpRequest &other);
	~HttpRequest();

	// 基本操作
	void				parse(const std::string &raw_data);
	bool				isCompleted() const;
	bool				isError() const;
	void				clear();

	// アクセサ
	const std::string&	getMethod() const;
	const std::string&	getUri() const;
	const std::string&	getPath() const;
	const std::string&	getQuery() const;
	const std::string&	getVersion() const;
	const std::string&	getHeader(const std::string& key) const;
	const std::map<std::string, std::string>&	getHeaders() const;
	const std::string&	getBody() const;
	int					getErrorCode() const;

	// 設定用
	void				setMaxBodySize(size_t max);

	// 検証用
	bool				isMethodAllowed(const std::vector<std::string>& allowedMethods) const;

private:
	enum ParsingState
	{
		REQUEST_LINE,
		HEADERS,
		BODY,
		CHUNKED_BODY,
		TRAILERS,
		COMPLETE,
		ERROR
	};

	ParsingState						_state;
	std::string							_method;
	std::string							_uri;
	std::string							_path;
	std::string							_query;
	std::string							_version;
	std::map<std::string, std::string>	_headers;
	std::string							_body;
	std::string							_buffer;
	std::string							_empty;
	size_t								_chunkSize;

	// 追加項目：エラー管理と制限
	int									_errorCode;
	size_t								_maxBodySize;

	// 内部解析ヘルパー
	void	_parseRequestLine(std::string &line);
	void	_parseHeader(std::string &line);
	void	_normalizePath(); // パストラバーサル対策用
};

#endif
