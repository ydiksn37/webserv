#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include "HttpRequest.hpp"
#include "ServerContext.hpp"
# include <string>
# include <vector>
# include <map>
# include <sys/types.h>

class CgiHandler {
	public:
		CgiHandler();
		~CgiHandler();

		std::pair<int, int> execute(const HttpRequest& request, 
										const ServerContext& server, 
										const LocationContext& location,
										const std::string& script_path,
										const std::string& bin_path);

		pid_t	getPid() const;

	private:
		pid_t																_pid;
		int																	_pipe_in[2];
		int																	_pipe_out[2];
		std::map<std::string, std::string>	_env;
		std::vector<std::string>						_env_strings;
		std::vector<char*>									_envp;
		std::vector<char*>									_argv;

		void	_setupEnv(const HttpRequest& request, 
										const ServerContext& server, 
										const LocationContext& location,
										const std::string& script_path);
		char**	_getEnvp();
		char**	_getArgv(const std::string& bin_path, const std::string& script_path);
};

#endif
