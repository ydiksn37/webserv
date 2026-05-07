#ifndef ENGINE_HPP
# define ENGINE_HPP

# include "HttpResponse.hpp"
# include "HttpRequest.hpp"
# include "Config.hpp"
# include <string>

struct EngineResult {
	HttpResponse						response;
	bool										is_cgi;
	std::string							script_path;
	std::string							bin_path;
	const ServerContext*		server;
	const LocationContext*	location;
	EngineResult();
};

EngineResult	engine(const Config &config, HttpRequest &request, int local_port);

#endif
