#include "engine.hpp"
#include <string>

std::string	engine(const Config &config, HttpRequest &request)
{
	HttpResponse response;
	(void)config;
	(void)request;

	return (response.serialize());
}
