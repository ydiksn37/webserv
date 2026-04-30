#include "engine.hpp"
#include <cstdlib>
#include <sstream>
#include <string>

struct RouteContext
{
	const ServerContext		*server;
	const LocationContext	*location;

	RouteContext() : server(NULL), location(NULL) {}
};

static void	applyErrorPage(HttpResponse &response, int code, const LocationContext *location, const ServerContext *server)
{
	response.setStatusCode(code);
	const std::map<int, std::string> *error_pages = NULL;

	if (location != NULL)
	{
		error_pages = &location->getErrorPages();
	}
	else if (server != NULL)
	{
		error_pages = &server->getErrorPages();
	}

	if (error_pages != NULL)
	{
		std::map<int, std::string>::const_iterator it = error_pages->find(code);
		if (it != error_pages->end())
		{
			response.setBodyFromFile(it->second);
		}
	}
}

static HttpResponse	buildPlaceholderResponse(const HttpRequest &request,
	const ServerContext *server,
	const LocationContext *location,
	const std::string &phase)
{
	HttpResponse		response;
	std::ostringstream	body;

	body << "<html>\r\n";
	body << "<head><title>Webserv Engine</title></head>\r\n";
	body << "<body>\r\n";
	body << "<h1>Engine bridge is active</h1>\r\n";
	body << "<p>Phase: " << phase << "</p>\r\n";
	body << "<p>Method: " << request.getMethod() << "</p>\r\n";
	body << "<p>Path: " << request.getPath() << "</p>\r\n";
	body << "<p>Version: " << request.getVersion() << "</p>\r\n";
	if (server != NULL)
	{
		body << "<p>Server port: " << server->getPort() << "</p>\r\n";
	}
	if (location != NULL)
	{
		body << "<p>Location: " << location->getPath() << "</p>\r\n";
	}
	body << "</body>\r\n";
	body << "</html>\r\n";
	response.setHeader("Content-Type", "text/html");
	response.setBody(body.str());
	return (response);
}

static HttpResponse	buildMethodNotAllowedResponse(const LocationContext &location)
{
	HttpResponse	response;
	std::string		allow;

	if (location.getIsMethodAllowed("GET")) allow += "GET";
	if (location.getIsMethodAllowed("POST")) { if (!allow.empty()) allow += ", "; allow += "POST"; }
	if (location.getIsMethodAllowed("DELETE")) { if (!allow.empty()) allow += ", "; allow += "DELETE"; }

	response.setStatusCode(405);
	response.setHeader("Allow", allow);
	return (response);
}

static std::string	extractHostName(const HttpRequest &request)
{
	std::string	host = request.getHeader("host");
	size_t		colon = host.find(':');

	if (colon == std::string::npos)
	{
		return (host);
	}
	return (host.substr(0, colon));
}

static int	extractPort(const HttpRequest &request)
{
	std::string	host = request.getHeader("host");
	size_t		colon = host.find(':');
	char		*endptr;
	long		port;

	if (colon == std::string::npos)
	{
		return (80);
	}
	port = std::strtol(host.substr(colon + 1).c_str(), &endptr, 10);
	if (*endptr != '\0' || port <= 0 || port > 65535)
	{
		return (80);
	}
	return (static_cast<int>(port));
}

static const ServerContext	*selectServer(const Config &config,
	const HttpRequest &request)
{
	return (config.getServer(extractPort(request), extractHostName(request)));
}

static const LocationContext	*selectLocation(const Config &config,
	const ServerContext *server,
	const HttpRequest &request)
{
	return (config.matchLocation(server, request.getPath()));
}

static bool	resolveRoute(const Config &config,
	HttpRequest &request,
	RouteContext &route,
	HttpResponse &response)
{
	route.server = selectServer(config, request);
	if (route.server == NULL)
	{
		applyErrorPage(response, 404, NULL, NULL);
		return (false);
	}
	route.location = selectLocation(config, route.server, request);
	if (route.location == NULL)
	{
		applyErrorPage(response, 404, NULL, route.server);
		return (false);
	}
	request.setMaxBodySize(route.location->getClientMaxBodySize());
	return (true);
}

static bool	prepareResponse(const HttpRequest &request,
	const RouteContext &route,
	HttpResponse &response)
{
	if (route.location->getRedirectCode() != 0)
	{
		response.setRedirect(route.location->getRedirectCode(), route.location->getRedirectUrl());
		return (false);
	}
	if (!route.location->getIsMethodAllowed(request.getMethod()))
	{
		response = buildMethodNotAllowedResponse(*route.location);
		applyErrorPage(response, 405, route.location, route.server);
		return (false);
	}
	return (true);
}

static HttpResponse	handleGet(const HttpRequest &request,
	const ServerContext &server,
	const LocationContext &location)
{
	return (buildPlaceholderResponse(request, &server, &location, "GET"));
}

static HttpResponse	handlePost(const HttpRequest &request,
	const ServerContext &server,
	const LocationContext &location)
{
	return (buildPlaceholderResponse(request, &server, &location, "POST"));
}

static HttpResponse	handleDelete(const HttpRequest &request,
	const ServerContext &server,
	const LocationContext &location)
{
	return (buildPlaceholderResponse(request, &server, &location, "DELETE"));
}

static HttpResponse	dispatchMethod(const HttpRequest &request,
	const ServerContext &server,
	const LocationContext &location)
{
	if (request.getMethod() == "GET")
	{
		return (handleGet(request, server, location));
	}
	if (request.getMethod() == "POST")
	{
		return (handlePost(request, server, location));
	}
	if (request.getMethod() == "DELETE")
	{
		return (handleDelete(request, server, location));
	}
	
	HttpResponse response;
	applyErrorPage(response, 501, &location, &server);
	return (response);
}

HttpResponse	engine(const Config &config, HttpRequest &request)
{
	RouteContext	route;
	HttpResponse	response;

	response.setHttpVersion(request.getVersion());

	if (request.isError())
	{
		applyErrorPage(response, request.getErrorCode(), NULL, NULL);
		return (response);
	}

	if (request.isHeaderFinished())
	{
		if (!resolveRoute(config, request, route, response))
		{
			return (response);
		}
		if (!prepareResponse(request, route, response))
		{
			return (response);
		}
		request.setRoutingResolved(true);
	}

	if (!request.isCompleted())
	{
		return (response);
	}

	return (dispatchMethod(request, *route.server, *route.location));
}
