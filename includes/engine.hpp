#ifndef ENGINE_HPP
# define ENGINE_HPP

# include "../srcs/Http/HttpResponse.hpp"
# include "../srcs/Http/HttpRequest.hpp"
# include "Config.hpp"
# include <string>

HttpResponse	engine(const Config &config, HttpRequest &request);

#endif
