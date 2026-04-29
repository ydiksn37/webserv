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

static HttpResponse	buildStatusResponse(int status_code)
{
	HttpResponse	response;

	response.setStatusCode(status_code);
	return (response);
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

static HttpResponse	buildMethodNotAllowedResponse(void)
{
	HttpResponse	response;

	response.setStatusCode(405);
	response.setHeader("Allow", "GET, POST, DELETE");
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

static HttpResponse	buildRedirectResponse(const LocationContext &location)
{
	HttpResponse	response;

	response.setRedirect(location.getRedirectCode(), location.getRedirectUrl());
	return (response);
}

static bool	parseRequest(const std::string &raw_request,
	HttpRequest &request,
	HttpResponse &response)
{
	request.parse(raw_request);
	if (request.isError())
	{
		response = buildStatusResponse(request.getErrorCode());
		return (false);
	}
	if (!request.isCompleted())
	{
		response = buildStatusResponse(400);
		return (false);
	}
	return (true);
}

static bool	resolveRoute(const Config &config,
	const HttpRequest &request,
	RouteContext &route,
	HttpResponse &response)
{
	route.server = selectServer(config, request);
	if (route.server == NULL)
	{
		response.setStatusCode(404);
		return (false);
	}
	route.location = selectLocation(config, route.server, request);
	if (route.location == NULL)
	{
		response.setStatusCode(404);
		return (false);
	}
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
		response = buildMethodNotAllowedResponse();
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
	return (buildStatusResponse(501));
}

std::string	Engine(const Config &config, const std::string& raw_request)
{
	HttpRequest		request;

	request.parse(raw_request);
	return (engine(config, request).serialize());
}

HttpResponse	engine(const Config &config, const HttpRequest &request)
{
	RouteContext	route;
	HttpResponse	response;

	if (request.isError())
	{
		response.setStatusCode(request.getErrorCode());
		return (response);
	}
	if (!request.isCompleted())
	{
		response.setStatusCode(400);
		return (response);
	}
	if (!resolveRoute(config, request, route, response))
	{
		return (response);
	}
	if (!prepareResponse(request, route, response))
	{
		return (response);
	}
	return (dispatchMethod(request, *route.server, *route.location));
}
