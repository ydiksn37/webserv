#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

#include <cstddef>
# include <string>
# include <map>

class HttpRequest
{
public:
	HttpRequest();
	HttpRequest(const HttpRequest &other);
	HttpRequest	&operator=(const HttpRequest &other);
	~HttpRequest();

	void	parse(const std::string &raw_data);
	bool	isCompleted() const;
	void	clear();

	const std::string	&getMethod() const;
	const std::string	&getUri() const;
	const std::string	&getPath() const;
	const std::string	&getQuery() const;
	const std::string	&getVersion() const;
	const std::string	&getHeader(const std::string& key) const;
	const std::string	&getBody() const;

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
	size_t								_chunkSize;

	void	_parseRequestLine(std::string &line);
	void	_parseHeader(std::string &line);
};

#endif
