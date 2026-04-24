#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

# include <string>
# include <map>

class HttpResponse
{
public:
	HttpResponse();
	HttpResponse(const HttpResponse& other);
	HttpResponse&	operator=(const HttpResponse& other);
	~HttpResponse();
		void			setStatusCode(int code);
		void			setReasonPhrase(const std::string& phrase);
		void			setHeader(const std::string& key, const std::string& value);
		void			setBody(const std::string& body);
		void			setHttpVersion(const std::string& version);

		std::string		serialize() const;

	private:
		std::string							_version;
		int									_statusCode;
		std::string							_reasonPhrase;
		std::map<std::string, std::string>	_headers;
		std::string							_body;

		static std::map<int, std::string>	_initStatusMessages();
		std::string							_getStatusMessage(int code) const;
		std::string							_getCurrentDate() const;
		std::string							_normalizeHeaderKey(const std::string& key) const;
		std::string							_generateDefaultErrorPage(int code) const;

	public:
		static std::string					generateDirectoryListing(const std::string& path, const std::string& uri);
		static std::string					getContentType(const std::string& path);
};

#endif
