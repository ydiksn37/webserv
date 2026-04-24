#include "../includes/config/ServerContext.hpp"
#include "../includes/config/Config.hpp"

void testRouting(const Config& config, int port, const std::string& host) {
	const ServerContext* server = config.getServer(port, host);
	std::cout << "[Test] Port: " << port << " | Host: " << host;
	if (server) {
		std::cout << " -> Matched: (Primary Name: " << *server->getServerNames().begin() << ")" << std::endl;
	} else {
		std::cout << " -> No Match" << std::endl;
	}
}

int main() {
	Config config;
	if (!config.loadFile("configurations/example.conf")) return 1;

	const std::vector<ServerContext>& servers = config.getServers();
	std::cout << "--- Registered Server Names ---" << std::endl;
	for (size_t i = 0; i < servers.size(); ++i) {
		std::cout << "Server " << i + 1 << " (Port " << servers[i].getPort() << "):";
		const std::set<std::string>& names = servers[i].getServerNames();
		for (std::set<std::string>::const_iterator it = names.begin(); it != names.end(); ++it) {
			std::cout << " " << *it;
		}
		std::cout << std::endl;
	}

	std::cout << "\n--- Routing Test ---" << std::endl;
	testRouting(config, 8080, "localhost");
	testRouting(config, 8080, "www.localhost");
	return 0;
}
