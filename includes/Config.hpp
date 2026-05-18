#ifndef CONFIG_HPP
# define CONFIG_HPP

# include "ServerContext.hpp"
# include <string>
# include <vector>
# include <cstdlib>
# include <stdexcept>
# include <cctype>
# include <cerrno>

class Config {
	public:
		Config();
		~Config();
		Config(const Config& other);
		Config& operator=(const Config& other);

		const std::vector<ServerContext>&	getServers() const;

		std::vector<std::string>	tokenize(const std::string& content);
		void											loadFile(const std::string& filename);
		const ServerContext*			getServer(int port, const std::string& host_name) const;
		const LocationContext*		matchLocation(const ServerContext* server, const std::string& uri) const;

		class ConfigException : public std::runtime_error {
			public:
				ConfigException(const std::string& msg) : std::runtime_error(msg) {}
		};

		private:
			std::vector<ServerContext>	_servers;
			void					parseServerBlock(const std::vector<std::string>& tokens, std::size_t& i);
			void					parseLocationBlock(const std::vector<std::string>& tokens, std::size_t& i, ServerContext& server);
			void					validateConfiguration() const;
			long					parseLong(const std::string& str) const;
			unsigned long	parseUnsignedLong(const std::string& str) const;
			void 					handleListen(const std::vector<std::string>& tokens, std::size_t& i, ServerContext& server);
			void 					handleServerName(const std::vector<std::string>& tokens, std::size_t& i, ServerContext& server);
			void 					handleRoot(const std::vector<std::string>& tokens, std::size_t& i, ServerContext& server);
			void 					handleRoot(const std::vector<std::string>& tokens, std::size_t& i, LocationContext& location);
			void 					handleIndex(const std::vector<std::string>& tokens, std::size_t& i, ServerContext& server, bool& index_specified);
			void 					handleIndex(const std::vector<std::string>& tokens, std::size_t& i, LocationContext& location, bool& index_specified);
			void 					handleClientMaxBodySize(const std::vector<std::string>& tokens, std::size_t& i, ServerContext& server);
			void 					handleClientMaxBodySize(const std::vector<std::string>& tokens, std::size_t& i, LocationContext& location);
			void 					handleErrorPage(const std::vector<std::string>& tokens, std::size_t& i, ServerContext& server);
			void 					handleErrorPage(const std::vector<std::string>& tokens, std::size_t& i, LocationContext& location);
			void 					handleAllowMethods(const std::vector<std::string>& tokens, std::size_t& i, LocationContext& location, bool& allowed_methods_specified);
			void 					handleAlias(const std::vector<std::string>& tokens, std::size_t& i, LocationContext& location, bool& alias_specified);
			void 					handleAutoindex(const std::vector<std::string>& tokens, std::size_t& i, LocationContext& location);
			void 					handleReturn(const std::vector<std::string>& tokens, std::size_t& i, LocationContext& location, bool& redirect_specified);
			void 					handleUploadEnable(const std::vector<std::string>& tokens, std::size_t& i, LocationContext& location);
			void 					handleUploadStore(const std::vector<std::string>& tokens, std::size_t& i, LocationContext& location);
			void 					handleCgiExtension(const std::vector<std::string>& tokens, std::size_t& i, LocationContext& location);
			void 					handleCgiPath(const std::vector<std::string>& tokens, std::size_t& i, LocationContext& location);
};

#endif
