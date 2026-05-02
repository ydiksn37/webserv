#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "Config.hpp"
# include "HttpRequest.hpp"
# include "CgiHandler.hpp"
# include <map>
# include <string>
# include <sys/epoll.h>
# include <ctime>

class Client {
	public:
		Client(const Config& config);
		struct PipeInfo {
			int fd;
			uint32_t events;
			PipeInfo(int f, uint32_t e);
		};

		int Read(int fd, std::vector<PipeInfo>& new_pipes);
		void Write(int fd);
		bool WriteBegin(int fd);
		bool WriteEnd(int fd);
		int  ReadCgi(int pipe_fd, bool closed_event);
		int  WriteCgi(int pipe_fd);
		int  GetClientFdFromPipe(int pipe_fd);
		std::vector<int> HandleCgiTimeout();
		void CleanupClient(int client_fd);

		void SetLocalPort(int fd, int port);
		bool ShouldClose(int fd);

	private:
		static const unsigned buffer_size = 8096;
		void GenResponse(int fd, std::string request);
		Config config_;
		struct ClientData {
			HttpRequest request;
			std::string write_buffer;
			unsigned current_epoll_events;
			bool		is_waiting_cgi;
			int			cgi_read_fd;
			int			cgi_write_fd;
			pid_t		cgi_pid;
			std::string	cgi_input;
			std::string	cgi_output;
			time_t		cgi_start_time;
			int			local_port;
			bool		should_close;
			ClientData();
		};
		std::map<int,ClientData> client_;
		std::map<int, int> pipe_to_client_;
};

#endif
