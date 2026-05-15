#include "CgiHandler.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <utility>
#include <limits.h>

CgiHandler::CgiHandler() : _pid(-1) {
	_pipe_in[0] = -1;
	_pipe_in[1] = -1;
	_pipe_out[0] = -1;
	_pipe_out[1] = -1;
}

CgiHandler::~CgiHandler() {
	if (_pipe_in[0] != -1) close(_pipe_in[0]);
	if (_pipe_in[1] != -1) close(_pipe_in[1]);
	if (_pipe_out[0] != -1) close(_pipe_out[0]);
	if (_pipe_out[1] != -1) close(_pipe_out[1]);
}

namespace {
	std::string sizeToStr(size_t size) {
		std::stringstream ss;
		ss << size;
		return ss.str();
	}
}

void CgiHandler::_setupEnv(const HttpRequest& request, 
								const ServerContext& server, 
								const LocationContext& location,
								const std::string& script_path) {
	(void)location;

	char abs_path[PATH_MAX];
	std::string final_path = script_path;
	if (realpath(script_path.c_str(), abs_path)) {
		final_path = abs_path;
	}

	_env["REQUEST_METHOD"] = request.getMethod();
	_env["QUERY_STRING"] = request.getQuery();
	_env["CONTENT_LENGTH"] = sizeToStr(request.getBody().length());
	_env["CONTENT_TYPE"] = request.getHeader("content-type");
	_env["SCRIPT_NAME"] = request.getPath();
	_env["PATH_INFO"] = request.getPath();
	_env["SERVER_NAME"] = request.getHeader("host");

	std::stringstream ss;
	ss << server.getPort();
	_env["SERVER_PORT"] = ss.str();
	_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	_env["REMOTE_ADDR"] = "";
	_env["REDIRECT_STATUS"] = "200";
	_env["SCRIPT_FILENAME"] = final_path;

	const std::map<std::string, std::string>& headers = request.getHeaders();
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
		std::string key = it->first;
		std::replace(key.begin(), key.end(), '-', '_');
		std::transform(key.begin(), key.end(), key.begin(), ::toupper);
		_env["HTTP_" + key] = it->second;
	}
}

char** CgiHandler::_getEnvp() {
	_env_strings.clear();
	for (std::map<std::string, std::string>::iterator it = _env.begin(); it != _env.end(); ++it) {
		_env_strings.push_back(it->first + "=" + it->second);
	}
	_envp.clear();
	for (size_t i = 0; i < _env_strings.size(); ++i) {
		_envp.push_back(const_cast<char*>(_env_strings[i].c_str()));
	}
	_envp.push_back(NULL);
	return &_envp[0];
}

char** CgiHandler::_getArgv(const std::string& bin_path, const std::string& script_path) {
	_argv.clear();
	_argv.push_back(const_cast<char*>(bin_path.c_str()));
	_argv.push_back(const_cast<char*>(script_path.c_str()));
	_argv.push_back(NULL);
	return &_argv[0];
}

std::pair<int, int> CgiHandler::execute(const HttpRequest& request, 
								const ServerContext& server, 
								const LocationContext& location,
								const std::string& script_path,
								const std::string& bin_path) {
	if (pipe(_pipe_in) < 0 || pipe(_pipe_out) < 0) {
		if (_pipe_in[0] != -1) close(_pipe_in[0]);
		if (_pipe_in[1] != -1) close(_pipe_in[1]);
		_pipe_in[0] = -1;
		_pipe_in[1] = -1;
		return std::make_pair(-1, -1);
	}

	_setupEnv(request, server, location, script_path);

	_pid = fork();
	if (_pid < 0) {
		close(_pipe_in[0]);
		close(_pipe_in[1]);
		close(_pipe_out[0]);
		close(_pipe_out[1]);
		_pipe_in[0] = -1;
		_pipe_in[1] = -1;
		_pipe_out[0] = -1;
		_pipe_out[1] = -1;
		return std::make_pair(-1, -1);
	}

	if (_pid == 0) {
		dup2(_pipe_in[0], STDIN_FILENO);
		dup2(_pipe_out[1], STDOUT_FILENO);

		close(_pipe_in[1]);
		close(_pipe_out[0]);
		close(_pipe_in[0]);
		close(_pipe_out[1]);

		size_t last_slash = script_path.find_last_of('/');
		std::string script_arg = script_path;
		if (last_slash != std::string::npos) {
			if (chdir(script_path.substr(0, last_slash).c_str()) != 0) {
				// エラーハンドリング
				std::cerr << "CGI Error: chdir failed" << std::endl;
				exit(EXIT_FAILURE);
			}
			script_arg = script_path.substr(last_slash + 1);
		}

		char** argv = _getArgv(bin_path, script_arg);
		char** envp = _getEnvp();

		execve(argv[0], argv, envp);
		exit(1);
	}

	close(_pipe_in[0]);
	close(_pipe_out[1]);
	_pipe_in[0] = -1;
	_pipe_out[1] = -1;

	fcntl(_pipe_in[1], F_SETFL, O_NONBLOCK);
	fcntl(_pipe_out[0], F_SETFL, O_NONBLOCK);

	int read_fd = _pipe_out[0];
	int write_fd = _pipe_in[1];
	_pipe_out[0] = -1;
	_pipe_in[1] = -1;
	return std::make_pair(read_fd, write_fd);
}

pid_t CgiHandler::getPid() const {
	return _pid;
}
