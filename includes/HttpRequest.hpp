#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <cstddef>
# include <string>
# include <map>
# include <vector>

class HttpRequest {
	public:
		HttpRequest();
		HttpRequest(const HttpRequest &other);
		HttpRequest&	operator=(const HttpRequest &other);
		~HttpRequest();

		void				parse(const std::string &raw_data);
		bool				isCompleted() const;
		bool				isError() const;
		void				clear();

		const std::string&	getMethod() const;
		const std::string&	getUri() const;
		const std::string&	getPath() const;
		const std::string&	getQuery() const;
		const std::string&	getVersion() const;
		const std::string&	getHeader(const std::string& key) const;
		const std::map<std::string, std::string>&	getHeaders() const;
		const std::string&	getBody() const;
		int						getErrorCode() const;
		bool					isHeaderFinished() const;
		bool					isRoutingResolved() const;
		std::size_t		getContentLength() const;

		void				setMaxBodySize(std::size_t max);
		void				setRoutingResolved(bool resolved);

		bool				isMethodAllowed(const std::vector<std::string>& allowedMethods) const;

	private:
		enum ParsingState {
			REQUEST_LINE,
			HEADERS,
			BODY,
			CHUNKED_BODY,
			TRAILERS,
			COMPLETE,
			ERROR
		};

		ParsingState												_state;
		bool																_routingResolved;
		std::string													_method;
		std::string													_uri;
		std::string													_path;
		std::string													_query;
		std::string													_version;
		std::map<std::string, std::string>	_headers;
		std::string													_body;
		std::string													_buffer;
		std::string													_empty;
		std::size_t													_chunkSize;
		std::size_t													_contentLength;
		bool																_hasContentLength;
		int																	_errorCode;
		std::size_t													_maxBodySize;

		void	_setError(int errorCode);
		bool	_parseLineState();
		bool	_parseFixedBody();
		bool	_readChunkSize();
		bool	_appendChunkData();
		bool	_parseChunkedBody();
		bool	_parseTrailer();
		void	_finishHeaders();
		bool	_validateRequestLineLayout(const std::string &line,
					std::size_t &firstSpace, std::size_t &secondSpace);
		bool	_validateRequestTarget();
		void	_splitUri();
		bool	_validateHeaderName(const std::string &key);
		bool	_parseContentLengthHeader(const std::string &key,
					const std::string &value);
		bool	_validateHostHeader(const std::string &key,
					const std::string &value);
		void	_parseRequestLine(std::string &line);
		void	_parseHeader(std::string &line);
		void	_normalizePath();
};

#endif
