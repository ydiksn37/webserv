#include "../includes/config/ServerContext.hpp"
#include "../includes/config/Config.hpp"
#include <iostream>
#include <exception>

void simulateRequest(const Config& config, int port, const std::string& host, const std::string& uri) {
	std::cout << "\n[Request] Port: " << port << " | Host: " << host << " | URI: " << uri << std::endl;
	const ServerContext* server = config.getServer(port, host);
	if (server == NULL) {
		std::cout << "  -> Result: 404 Not Found (No Server on this Port)" << std::endl;
		return;
	}
	std::string primary_name = "(None)";
	if (!server->getServerNames().empty()) {
		primary_name = *server->getServerNames().begin();
	}
	std::cout << "  -> Matched Server : " << primary_name << " (Port: " << server->getPort() << ")" << std::endl;
	const LocationContext* location = config.matchLocation(server, uri);
	if (location == NULL) {
		std::cout << "  -> Matched Location: (None) -> 404 Not Found" << std::endl;
		return;
	}
	std::cout << "  -> Matched Location: " << location->getPath() << std::endl;
	std::cout << "  -> Effective Config:" << std::endl;
	std::cout << "       |- Max Body Size: " << location->getClientMaxBodySize() << std::endl;
	
	const std::map<int, std::string>& errors = location->getErrorPages();
	if (errors.find(404) != errors.end()) {
		std::cout << "       |- Error Page 404: " << errors.find(404)->second << std::endl;
	}
	if (!location->getAlias().empty()) {
		std::string remaining = uri.substr(location->getPath().length());
		std::cout << "  -> Target File    : " << location->getAlias() << remaining << " (Alias applied)" << std::endl;
	} else if (!location->getRoot().empty()) {
		std::cout << "  -> Target File    : " << location->getRoot() << uri << " (Root applied)" << std::endl;
	} else {
		std::cout << "  -> Target File    : (No Root/Alias set)" << std::endl;
	}
}

int main(int argc, char **argv) {
	std::string config_file = (argc >= 2) ? argv[1] : "configurations/example.conf";
	Config config;

	std::cout << "Loading " << config_file << "..." << std::endl;

	try {
		config.loadFile(config_file);
		std::cout << "[Success] Configuration parsed and validated perfectly!\n" << std::endl;
	} catch (const Config::ConfigException& e) {
		std::cerr << "\n[Config Error] " << e.what() << std::endl;
		return 1;
	} catch (const std::exception& e) {
		std::cerr << "\n[Validation Error] " << e.what() << std::endl;
		return 1;
	}

	std::cout << "========================================" << std::endl;
	std::cout << "      Routing Engine Simulation         " << std::endl;
	std::cout << "========================================" << std::endl;
	simulateRequest(config, 8080, "example.com", "/");
	simulateRequest(config, 8080, "localhost", "/");
	simulateRequest(config, 8080, "localhost", "/upload/test.png");
	simulateRequest(config, 8080, "localhost", "/kapouet/pouic/toto.html");
	simulateRequest(config, 8080, "localhost", "/uploads/dummy.txt");

	return 0;
}
