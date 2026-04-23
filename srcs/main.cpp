#include "../includes/config/ServerContext.hpp"
#include "../includes/config/Config.hpp"

std::string getAllowedMethodsString(const LocationContext& loc) {
	std::string methods = "";
	if (loc.getIsMethodAllowed("GET")) methods += "GET ";
	if (loc.getIsMethodAllowed("POST")) methods += "POST ";
	if (loc.getIsMethodAllowed("DELETE")) methods += "DELETE ";
	return methods.empty() ? "(None)" : methods;
}

int main(int argc, char **argv) {
	std::string config_file = (argc >= 2) ? argv[1] : "configurations/example.conf";

	Config config;

	std::cout << "Loading config: " << config_file << std::endl;
	if (!config.loadFile(config_file)) {
		std::cerr << "Failed to load configuration file." << std::endl;
		return 1;
	}

	const std::vector<ServerContext>& servers = config.getServers();
	std::cout << "\n==================================================" << std::endl;
	std::cout << "             Parsed Configuration                 " << std::endl;
	std::cout << "==================================================" << std::endl;
	std::cout << "Total Servers: " << servers.size() << "\n" << std::endl;

	for (size_t i = 0; i < servers.size(); ++i) {
		std::cout << "[Server " << i + 1 << "]" << std::endl;
		std::cout << "  |- Port: " << servers[i].getPort() << std::endl;
		std::cout << "  |- Name: " << servers[i].getServerName() << std::endl;
		std::cout << "  |- Max Body Size: " << servers[i].getClientMaxBodySize() << std::endl;

		const std::map<int, std::string>& error_pages = servers[i].getErrorPages();
		if (!error_pages.empty()) {
			std::cout << "  |- Error Pages:" << std::endl;
			for (std::map<int, std::string>::const_iterator it = error_pages.begin(); it != error_pages.end(); ++it) {
				std::cout << "       " << it->first << " -> " << it->second << std::endl;
			}
		} else {
			std::cout << "  |- Error Pages: (None)" << std::endl;
		}

		const std::vector<LocationContext>& locations = servers[i].getLocations();
		if (!locations.empty()) {
			std::cout << "  |- Locations (" << locations.size() << "):" << std::endl;
			for (size_t j = 0; j < locations.size(); ++j) {
				const LocationContext& loc = locations[j];
				std::cout << "       [" << j + 1 << "] Path: " << loc.getPath() << std::endl;
				std::cout << "             |- Allowed Methods : " << getAllowedMethodsString(loc) << std::endl;
				std::cout << "             |- Root            : " << (loc.getRoot().empty() ? "(None)" : loc.getRoot()) << std::endl;
				std::cout << "             |- Alias           : " << (loc.getAlias().empty() ? "(None)" : loc.getAlias()) << std::endl;

				const std::vector<std::string>& indexes = loc.getIndex();
				std::cout << "             |- Index           : ";
				if (indexes.empty()) {
					std::cout << "(None)";
				} else {
					for (size_t k = 0; k < indexes.size(); ++k) {
						std::cout << indexes[k] << (k + 1 < indexes.size() ? ", " : "");
					}
				}
				std::cout << std::endl;

				std::cout << "             |- Autoindex       : " << (loc.getAutoindex() ? "ON" : "OFF") << std::endl;
				
				if (loc.getRedirectCode() != 0) {
					std::cout << "             |- Redirect        : " << loc.getRedirectCode() << " -> " << loc.getRedirectUrl() << std::endl;
				}

				std::cout << "             |- Upload Enable   : " << (loc.getUploadEnable() ? "ON" : "OFF") << std::endl;
				if (!loc.getUploadStore().empty()) {
					std::cout << "             |- Upload Store    : " << loc.getUploadStore() << std::endl;
				}

				const std::map<std::string, std::string>& cgi_info = loc.getCgiInfo();
				if (!cgi_info.empty()) {
					std::cout << "             |- CGI Info        :" << std::endl;
					for (std::map<std::string, std::string>::const_iterator it = cgi_info.begin(); it != cgi_info.end(); ++it) {
						std::cout << "                  " << it->first << " -> " << it->second << std::endl;
					}
				}
			}
		} else {
			std::cout << "  |- Locations: (None)" << std::endl;
		}
		std::cout << "--------------------------------------------------\n" << std::endl;
	}

	return 0;
}
