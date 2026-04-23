#include "../includes/config/ServerContext.hpp"
#include "../includes/config/Config.hpp"

int main(int argc, char **argv) {
	std::string config_file = (argc >= 2) ? argv[1] : "configurations/test.conf";

	Config config;

	std::cout << "Loading config: " << config_file << std::endl;
	if (!config.loadFile(config_file)) {
		std::cerr << "Failed to load configuration file." << std::endl;
		return 1;
	}

	// パース結果の取得
	const std::vector<ServerContext>& servers = config.getServers();
	std::cout << "\n========================================" << std::endl;
	std::cout << "         Parsed Configuration           " << std::endl;
	std::cout << "========================================" << std::endl;
	std::cout << "Total Servers: " << servers.size() << "\n" << std::endl;

	for (size_t i = 0; i < servers.size(); ++i) {
		std::cout << "[Server " << i + 1 << "]" << std::endl;
		std::cout << "  |- Port: " << servers[i].getPort() << std::endl;
		std::cout << "  |- Name: " << servers[i].getServerName() << std::endl;
		std::cout << "  |- Max Body Size: " << servers[i].getClientMaxBodySize() << std::endl;

		// エラーページの表示（C++98準拠のイテレータ使用）
		const std::map<int, std::string>& error_pages = servers[i].getErrorPages();
		if (!error_pages.empty()) {
			std::cout << "  |- Error Pages:" << std::endl;
			for (std::map<int, std::string>::const_iterator it = error_pages.begin(); it != error_pages.end(); ++it) {
				std::cout << "       " << it->first << " -> " << it->second << std::endl;
			}
		} else {
			std::cout << "  |- Error Pages: (None)" << std::endl;
		}

		// Locationブロックの表示
		const std::vector<LocationContext>& locations = servers[i].getLocations();
		if (!locations.empty()) {
			std::cout << "  |- Locations (" << locations.size() << "):" << std::endl;
			for (size_t j = 0; j < locations.size(); ++j) {
				std::cout << "       - Path: " << locations[j].getPath() << std::endl;
			}
		} else {
			std::cout << "  |- Locations: (None)" << std::endl;
		}
		std::cout << "----------------------------------------\n" << std::endl;
	}

	return 0;
}
