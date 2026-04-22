#include "../includes/config/Config.hpp"

int main() {
	std::string raw_config = "server { listen 8080; server_name example.com; }";

	std::vector<std::string> tokens = tokenize(raw_config);

	for (size_t i = 0; i < tokens.size(); ++i) {
		std::cout << "[" << tokens[i] << "]" << std::endl;
	}

	return 0;
}
