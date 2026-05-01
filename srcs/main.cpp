#include "Config.hpp"
#include "EventLoop.hpp"

int main(int argc, char** argv) {
	std::string config_file;

	if (argc == 2) {
		config_file = argv[1];
	} else if (argc == 1) {
		config_file = "configurations/default.conf";
		std::cout << "[Info] No configuration file specified. Using default: " << config_file << std::endl;
	} else {
		std::cerr << "Usage: ./webserv [config_file]" << std::endl;
		return 1;
	}

	Config config;

	try {
		config.loadFile(config_file);
	} catch (const Config::ConfigException& e) {
		std::cerr << "\n[Config Error] " << e.what() << std::endl;
		return 1;
	} catch (const std::exception& e) {
		std::cerr << "\n[Validation Error] " << e.what() << std::endl;
		return 1;
	}

	try {
		EventLoop(config);
	} catch (const std::exception& e) {
		std::cerr << "\n[Error] " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
