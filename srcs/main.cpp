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

	const std::vector<ServerContext>& servers = config.getServers();
	std::cout << "\n--- Parsed Configuration ---" << std::endl;
	std::cout << "Total Servers: " << servers.size() << std::endl;
	
	for (size_t i = 0; i < servers.size(); ++i) {
		std::cout << "[Server " << i + 1 << "]" << std::endl;
		std::cout << "  Port: " << servers[i].getPort() << std::endl;
		std::cout << "  Name: " << servers[i].getServerName() << std::endl;
	}
	std::cout << "----------------------------\n" << std::endl;

	return 0;
}
