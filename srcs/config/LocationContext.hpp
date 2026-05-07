#ifndef LOCATIONCONTEXT_HPP
# define LOCATIONCONTEXT_HPP

# include <iostream>
# include <string>
# include <vector>
# include <map>

enum HttpMethod {
	METHOD_GET    = 1 << 0,
	METHOD_POST   = 1 << 1,
	METHOD_DELETE = 1 << 2
};

class LocationContext {
	private:
		std::string													_path;
		unsigned char												_allowed_methods;
		std::string													_root;
		std::string													_alias;
		std::vector<std::string>						_index;
		bool																_autoindex;
		int																	_redirect_code;
		std::string													_redirect_url;
		bool																_upload_enable;
		std::string													_upload_store;
		std::vector<std::string>						_cgi_extensions;
		std::map<std::string, std::string>	_cgi_info;
		size_t															_client_max_body_size;
		std::map<int, std::string>					_error_pages;

	public:
		LocationContext();
		~LocationContext();
		LocationContext(const LocationContext& other);
		LocationContext& operator=(const LocationContext& other);

		LocationContext(const std::string& path);

		void setPath(const std::string& path);
		void setAllowedMethod(const std::string& method);
		void clearAllowedMethods();
		void setRoot(const std::string& root);
		void setAlias(const std::string& alias);
		void setIndex(const std::string& index);
		void clearIndex();
		void setAutoindex(bool autoindex);
		void setRedirect(int code, const std::string& url);
		void setUploadEnable(bool enable);
		void setUploadStore(const std::string& store);
		void setCgiExtension(const std::string& ext);
		void setCgiPath(const std::string& ext, const std::string& path);
		void setClientMaxBodySize(size_t size);
		void setErrorPage(int status_code, const std::string& path);

		const std::string&												getPath() const;
		bool																			getIsMethodAllowed(const std::string& method) const;
		const std::string&												getRoot() const;
		const std::string&												getAlias() const;
		const std::vector<std::string>&						getIndex() const;
		bool																			getAutoindex() const;
		int																				getRedirectCode() const;
		const std::string&												getRedirectUrl() const;
		bool																			getUploadEnable() const;
		const std::string&												getUploadStore() const;
		const std::vector<std::string>&						getCgiExtensions() const;
		const std::map<std::string, std::string>&	getCgiInfo() const;
		size_t																		getClientMaxBodySize() const;
		const std::map<int, std::string>&					getErrorPages() const;

		void validate() const;
};

#endif
