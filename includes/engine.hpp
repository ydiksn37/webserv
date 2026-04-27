#ifndef ENGINE_HPP
# define ENGINE_HPP

# include "../srcs/Http/HttpResponse.hpp"
# include "../srcs/Http/HttpRequest.hpp"
# include "Config.hpp"
#include <string>

std::string		Engine(const Config &config, const std::string& raw_request);
HttpResponse	engine(const Config &config, const HttpRequest &request);

#endif
