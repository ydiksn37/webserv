#include "engine.hpp"
#include <cstdlib>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

struct RouteContext {
	const ServerContext		*server;
	const LocationContext	*location;

	RouteContext() : server(NULL), location(NULL) {}
};

EngineResult::EngineResult() : is_cgi(false), server(NULL), location(NULL) {}

static bool fileExists(const std::string& path) {
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0);
}

static bool isDirectory(const std::string& path) {
	struct stat buffer;
	if (stat(path.c_str(), &buffer) != 0)
		return false;
	return S_ISDIR(buffer.st_mode);
}

static void	applyErrorPage(HttpResponse &response, int code, const LocationContext *location, const ServerContext *server) {
	response.setStatusCode(code);
	const std::map<int, std::string> *error_pages = NULL;

	if (location != NULL)
		error_pages = &location->getErrorPages();
	else if (server != NULL)
		error_pages = &server->getErrorPages();

	if (error_pages != NULL) {
		std::map<int, std::string>::const_iterator it = error_pages->find(code);
		if (it != error_pages->end())
			response.setBodyFromFile(it->second);
	}
}

static HttpResponse	buildMethodNotAllowedResponse(const LocationContext &location) {
	HttpResponse	response;
	std::string		allow;

	if (location.getIsMethodAllowed("GET")) allow += "GET";
	if (location.getIsMethodAllowed("POST")) { if (!allow.empty()) allow += ", "; allow += "POST"; }
	if (location.getIsMethodAllowed("DELETE")) { if (!allow.empty()) allow += ", "; allow += "DELETE"; }

	response.setStatusCode(405);
	response.setHeader("Allow", allow);
	return (response);
}

static std::string	extractHostName(const HttpRequest &request) {
	std::string	host = request.getHeader("host");
	size_t		colon = host.find(':');

	if (colon == std::string::npos)
		return (host);
	return (host.substr(0, colon));
}

static const ServerContext	*selectServer(const Config &config, const HttpRequest &request, int local_port) {
	std::string host = extractHostName(request);
	return (config.getServer(local_port, host));
}

static const LocationContext	*selectLocation(const Config &config, const ServerContext *server, const HttpRequest &request) {
	return (config.matchLocation(server, request.getPath()));
}

static bool	resolveRoute(const Config &config, HttpRequest &request, RouteContext &route, HttpResponse &response, int local_port) {
	route.server = selectServer(config, request, local_port);
	if (route.server == NULL) {
		std::cerr << "[Debug] No server found for port " << local_port << " and host " << extractHostName(request) << std::endl;
		applyErrorPage(response, 404, NULL, NULL);
		return (false);
	}
	route.location = selectLocation(config, route.server, request);
	if (route.location == NULL) {
		std::cerr << "[Debug] No location found for path " << request.getPath() << std::endl;
		applyErrorPage(response, 404, NULL, route.server);
		return (false);
	}
	request.setMaxBodySize(route.location->getClientMaxBodySize());
	return (true);
}

static bool	prepareResponse(const HttpRequest &request, const RouteContext &route, HttpResponse &response) {
	if (route.location->getRedirectCode() != 0) {
		response.setRedirect(route.location->getRedirectCode(), route.location->getRedirectUrl());
		return (false);
	}
	if (!route.location->getIsMethodAllowed(request.getMethod())) {
		response = buildMethodNotAllowedResponse(*route.location);
		applyErrorPage(response, 405, route.location, route.server);
		return (false);
	}
	return (true);
}

static bool isCgi(const HttpRequest &request, const LocationContext &location, std::string &bin_path) {
	const std::map<std::string, std::string> &cgi_info = location.getCgiInfo();
	std::string path = request.getPath();
	size_t dot = path.find_last_of('.');
	if (dot == std::string::npos) return false;
	std::string ext = path.substr(dot);
	std::map<std::string, std::string>::const_iterator it = cgi_info.find(ext);
	if (it != cgi_info.end()) {
		bin_path = it->second;
		return true;
	}
	return false;
}

static std::string translatePath(const HttpRequest &request, const LocationContext &location, const ServerContext &server) {
	std::string root = location.getRoot();
	std::string alias = location.getAlias();
	std::string loc_path = location.getPath();
	std::string req_path = request.getPath();

	if (!alias.empty()) {
		return alias + req_path.substr(loc_path.length());
	}
	if (root.empty()) {
		root = server.getRoot();
	}
	return root + req_path;
}

static std::string resolveScriptPath(const HttpRequest &request, const LocationContext &location, const ServerContext &server) {
	return translatePath(request, location, server);
}

static HttpResponse	handleGet(const HttpRequest &request, const ServerContext &server, const LocationContext &location) {
	HttpResponse response;
	std::string full_path = translatePath(request, location, server);

	if (!fileExists(full_path)) {
		applyErrorPage(response, 404, &location, &server);
		return response;
	}

	if (isDirectory(full_path)) {
		if (request.getPath()[request.getPath().length() - 1] != '/') {
			response.setRedirect(301, request.getPath() + "/");
			return response;
		}

		const std::vector<std::string>& indexes = location.getIndex();
		for (size_t i = 0; i < indexes.size(); ++i) {
			std::string index_path = full_path + indexes[i];
			if (fileExists(index_path) && !isDirectory(index_path)) {
				response.setBodyFromFile(index_path);
				return response;
			}
		}

		if (location.getAutoindex()) {
			std::string listing = HttpResponse::generateDirectoryListing(full_path, request.getPath());
			if (!listing.empty()) {
				response.setStatusCode(200);
				response.setHeader("Content-Type", "text/html");
				response.setBody(listing);
				return response;
			}
		}

		applyErrorPage(response, 403, &location, &server);
		return response;
	}

	response.setBodyFromFile(full_path);
	return (response);
}

static bool ensureDirectory(const std::string& path) {
	struct stat st;
	if (stat(path.c_str(), &st) != 0) {
		// ディレクトリ作成（簡易版、再帰的ではない）
		if (mkdir(path.c_str(), 0755) != 0)
			return false;
	}
	else if (!S_ISDIR(st.st_mode)) {
		return false;
	}
	return true;
}

static std::string getFilenameFromPath(const std::string& path) {
	size_t pos = path.find_last_of('/');
	if (pos == std::string::npos)
		return path;
	return path.substr(pos + 1);
}

static HttpResponse	handlePost(const HttpRequest &request, const ServerContext &server, const LocationContext &location) {
	HttpResponse response;

	if (!location.getUploadEnable()) {
		applyErrorPage(response, 403, &location, &server);
		return response;
	}

	std::string store = location.getUploadStore();
	if (store.empty()) {
		applyErrorPage(response, 500, &location, &server);
		return response;
	}

	if (!ensureDirectory(store)) {
		applyErrorPage(response, 500, &location, &server);
		return response;
	}

	// ファイル名の決定
	// 1. URIから取得（例: /upload/filename.txt）
	// 2. なければ Content-Disposition 等から（今回は簡略化のためURI優先）
	std::string filename = getFilenameFromPath(request.getPath());
	if (filename.empty()) {
		// ディレクトリへのPOSTなどの場合、タイムスタンプ等で生成するか400を返す
		// 課題の要件に合わせ、URIにファイル名が含まれていることを期待する実装とする
		applyErrorPage(response, 400, &location, &server);
		return response;
	}

	std::string filepath = store;
	if (filepath[filepath.length() - 1] != '/')
		filepath += "/";
	filepath += filename;

	std::ofstream outfile(filepath.c_str(), std::ios::binary);
	if (!outfile.is_open()) {
		applyErrorPage(response, 500, &location, &server);
		return response;
	}

	outfile.write(request.getBody().c_str(), request.getBody().length());
	outfile.close();

	response.setStatusCode(201);
	response.setBody("<html><body><h1>File uploaded successfully</h1></body></html>");
	response.setHeader("Content-Type", "text/html");
	return (response);
}

static HttpResponse	handleDelete(const HttpRequest &request, const ServerContext &server, const LocationContext &location) {
	std::string full_path = translatePath(request, location, server);
	HttpResponse response;

	if (!fileExists(full_path)) {
		applyErrorPage(response, 404, &location, &server);
		return response;
	}
	if (isDirectory(full_path)) {
		applyErrorPage(response, 403, &location, &server);
		return response;
	}
	if (remove(full_path.c_str()) == 0)
		response.setStatusCode(204);
	else
		applyErrorPage(response, 500, &location, &server);
	return (response);
}

EngineResult	dispatchMethod(const HttpRequest &request, const ServerContext &server, const LocationContext &location) {
	EngineResult result;
	std::string bin_path;

	result.server = &server;
	result.location = &location;

	if (isCgi(request, location, bin_path)) {
		result.is_cgi = true;
		result.bin_path = bin_path;
		result.script_path = resolveScriptPath(request, location, server);
		return result;
	}

	if (request.getMethod() == "GET")
		result.response = handleGet(request, server, location);
	else if (request.getMethod() == "POST")
		result.response = handlePost(request, server, location);
	else if (request.getMethod() == "DELETE")
		result.response = handleDelete(request, server, location);
	else
		applyErrorPage(result.response, 501, &location, &server);
	return (result);
}

EngineResult	engine(const Config &config, HttpRequest &request, int local_port) {
	RouteContext	route;
	EngineResult	result;

	result.response.setHttpVersion(request.getVersion());

	if (request.isError()) {
		applyErrorPage(result.response, request.getErrorCode(), NULL, NULL);
		return (result);
	}

	if (request.isHeaderFinished()) {
		if (!resolveRoute(config, request, route, result.response, local_port))
			return (result);
		if (!prepareResponse(request, route, result.response))
			return (result);
		request.setRoutingResolved(true);
	}

	if (!request.isCompleted())
		return (result);

	return (dispatchMethod(request, *route.server, *route.location));
}
