#include <iostream>
#include "Config.hpp"
#include "EventLoop.hpp"

int main(int argc, char** argv)
{
	if(argc != 2)
		std::cerr<<"Invalid argument."<<std::endl;
	Config config;

	try {
		config.loadFile(argv[1]);
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
		std::cerr << "\n[Error]" << e.what() << std::endl;
		return 1;
	}
}
