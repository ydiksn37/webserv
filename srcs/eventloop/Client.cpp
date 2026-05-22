#include "Client.hpp"
#include "engine.hpp"
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <cctype>
#include <sstream>
#include <ctime>
#include "CgiHandler.hpp"

bool endswith(std::string str, std::string suffix) {
	if(str.size() < suffix.size())
		return false;
	return str.substr(str.size() - suffix.size()) == suffix;
}

namespace {
	std::string toLowerString(std::string str) {
		for (std::size_t i = 0; i < str.length(); ++i)
			str[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(str[i])));
		return str;
	}

	std::string sizeToString(std::size_t size) {
		std::stringstream ss;
		ss << size;
		return ss.str();
	}
}

Client::Client(const Config& config):config_(config) {}

Client::PipeInfo::PipeInfo(int f, uint32_t e) : fd(f), events(e) {}

Client::ClientData::ClientData()
    : current_epoll_events(EPOLLIN), is_waiting_cgi(false), 
      cgi_read_fd(-1), cgi_write_fd(-1), cgi_pid(-1),
      cgi_start_time(0), local_endpoint(), should_close(false) {}

void Client::SetLocalEndpoint(int fd, const ListenEndpoint& endpoint) {
	client_[fd].local_endpoint = endpoint;
}

bool Client::ShouldClose(int fd) {
	if (client_.count(fd))
		return client_[fd].should_close;
	return true;
}

int Client::Read(int fd, std::vector<PipeInfo>& new_pipes) {
	char tmp_buffer[buffer_size];
	int read_size = read(fd, tmp_buffer, buffer_size);
	if(read_size <= 0) {
		CleanupClient(fd);
		return -1;
	}
	client_[fd].request.parse(std::string(tmp_buffer,read_size));

	if (client_[fd].request.isHeaderFinished()
		&& !client_[fd].request.isRoutingResolved()
		&& !client_[fd].request.isCompleted()
		&& !client_[fd].request.isError()) {
		engine(config_, client_[fd].request, client_[fd].local_endpoint);
	}

	while((client_[fd].request.isCompleted() || client_[fd].request.isError()) && !client_[fd].is_waiting_cgi) {
		std::cerr << "[Debug] Processing request: " << client_[fd].request.getMethod() << " " << client_[fd].request.getPath() << " on " << client_[fd].local_endpoint.host << ":" << client_[fd].local_endpoint.port << std::endl;
		EngineResult res = engine(config_, client_[fd].request, client_[fd].local_endpoint);

		std::string conn = client_[fd].request.getHeader("connection");
		for (std::size_t i = 0; i < conn.length(); ++i) conn[i] = std::tolower(conn[i]);
		if (conn == "close" || res.response.getStatusCode() == 413 || res.response.getStatusCode() == 400)
			client_[fd].should_close = true;

		if (res.is_cgi) {
			CgiHandler cgi;
			std::pair<int, int> pipes = cgi.execute(client_[fd].request, *res.server, *res.location, res.script_path, res.bin_path);

			if (pipes.first != -1) {
				client_[fd].is_waiting_cgi = true;
				client_[fd].cgi_read_fd = pipes.first;
				client_[fd].cgi_write_fd = pipes.second;
				client_[fd].cgi_pid = cgi.getPid();
				client_[fd].cgi_input = client_[fd].request.getBody();
				client_[fd].cgi_start_time = std::time(NULL);
				client_[fd].cgi_output.clear();

				pipe_to_client_[pipes.first] = fd;
				new_pipes.push_back(PipeInfo(pipes.first, EPOLLIN));

				if (!client_[fd].cgi_input.empty()) {
					pipe_to_client_[pipes.second] = fd;
					new_pipes.push_back(PipeInfo(pipes.second, EPOLLOUT));
				}
				else {
					if (pipes.second != -1) {
						close(pipes.second);
						client_[fd].cgi_write_fd = -1;
					}
				}
			}
			else {
				HttpResponse err;
				err.setStatusCode(500);
				client_[fd].write_buffer.append(err.serialize());
			}
		}
		else {
			client_[fd].write_buffer.append(res.response.serialize());
		}
		client_[fd].request.clear();
		client_[fd].request.parse("");
	}
	return 0;
}

void Client::Write(int fd) {
	if (!client_.count(fd)) return;
	int write_size = write(fd, client_[fd].write_buffer.c_str(), client_[fd].write_buffer.size());
	if (write_size > 0)
		client_[fd].write_buffer.erase(0,write_size);
}

bool Client::WriteBegin(int fd) {
	if(client_[fd].current_epoll_events == EPOLLIN && client_[fd].write_buffer.size()) {
		client_[fd].current_epoll_events = EPOLLIN | EPOLLOUT;
		return true;
	}
	return false;
}

bool Client::WriteEnd(int fd) {
	if(client_[fd].current_epoll_events == (EPOLLIN | EPOLLOUT) && client_[fd].write_buffer.empty()) {
		client_[fd].current_epoll_events = EPOLLIN;
		return true;
	}
	return false;
}

int Client::ReadCgi(int pipe_fd, bool closed_event) {
	if (!pipe_to_client_.count(pipe_fd)) return -1;
	int client_fd = pipe_to_client_[pipe_fd];
	char buffer[buffer_size];
	bool reached_eof = false;

	int read_size = read(pipe_fd, buffer, buffer_size);
	if (read_size > 0) {
		client_[client_fd].cgi_output.append(buffer, read_size);
		if (!closed_event)
			return 0;
		while ((read_size = read(pipe_fd, buffer, buffer_size)) > 0) {
			client_[client_fd].cgi_output.append(buffer, read_size);
		}
	}
	if (read_size == 0) {
		reached_eof = true;
	}
	if (!reached_eof && !closed_event)
		return 0;

	std::string status_line = "200 OK";
	std::size_t status_pos = client_[client_fd].cgi_output.find("Status: ");
	if (status_pos != std::string::npos) {
		std::size_t end_of_line = client_[client_fd].cgi_output.find("\n", status_pos);
		if (end_of_line != std::string::npos) {
			status_line = client_[client_fd].cgi_output.substr(status_pos + 8, end_of_line - (status_pos + 8));
			if (!status_line.empty() && status_line[status_line.length() - 1] == '\r')
				status_line.erase(status_line.length() - 1);
			client_[client_fd].cgi_output.erase(status_pos, end_of_line + 1 - status_pos);
		}
	}

	client_[client_fd].write_buffer.append("HTTP/1.1 " + status_line + "\r\n");
	std::size_t header_end = client_[client_fd].cgi_output.find("\r\n\r\n");
	std::size_t separator_len = 4;
	if (header_end == std::string::npos) {
		header_end = client_[client_fd].cgi_output.find("\n\n");
		separator_len = 2;
	}
	if (header_end != std::string::npos) {
		std::string headers = client_[client_fd].cgi_output.substr(0, header_end);
		std::string body = client_[client_fd].cgi_output.substr(header_end + separator_len);
		client_[client_fd].write_buffer.append(headers + "\r\n");
		if (toLowerString(headers).find("content-length:") == std::string::npos)
			client_[client_fd].write_buffer.append("Content-Length: " + sizeToString(body.length()) + "\r\n");
		client_[client_fd].write_buffer.append("\r\n");
		client_[client_fd].write_buffer.append(body);
	}
	else {
		client_[client_fd].write_buffer.append("Content-Length: " + sizeToString(client_[client_fd].cgi_output.length()) + "\r\n");
		client_[client_fd].write_buffer.append("Content-Type: text/plain\r\n\r\n");
		client_[client_fd].write_buffer.append(client_[client_fd].cgi_output);
	}
	client_[client_fd].is_waiting_cgi = false;
	pipe_to_client_.erase(pipe_fd);
	client_[client_fd].cgi_read_fd = -1;
	if (client_[client_fd].cgi_pid != -1) {
		int status;
		if (waitpid(client_[client_fd].cgi_pid, &status, WNOHANG) > 0) {
			if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
				std::cerr << "[CGI Error] PID " << client_[client_fd].cgi_pid << " exited with " << WEXITSTATUS(status) << std::endl;
			}
		}
		client_[client_fd].cgi_pid = -1;
	}
	return 1;
}

int Client::WriteCgi(int pipe_fd) {
	if (!pipe_to_client_.count(pipe_fd)) return -1;
	int client_fd = pipe_to_client_[pipe_fd];

	int write_size = write(pipe_fd, client_[client_fd].cgi_input.c_str(), client_[client_fd].cgi_input.size());
	if (write_size > 0) {
		client_[client_fd].cgi_input.erase(0, write_size);
	}
	if (client_[client_fd].cgi_input.empty()) {
		pipe_to_client_.erase(pipe_fd);
		client_[client_fd].cgi_write_fd = -1;
		return 1;
	}
	return 0;
}

int Client::GetClientFdFromPipe(int pipe_fd) {
	if (pipe_to_client_.count(pipe_fd))
		return pipe_to_client_[pipe_fd];
	return -1;
}

void Client::CleanupClient(int client_fd) {
	if (client_.count(client_fd)) {
		if (client_[client_fd].cgi_read_fd != -1) {
			pipe_to_client_.erase(client_[client_fd].cgi_read_fd);
			close(client_[client_fd].cgi_read_fd);
		}
		if (client_[client_fd].cgi_write_fd != -1) {
			pipe_to_client_.erase(client_[client_fd].cgi_write_fd);
			close(client_[client_fd].cgi_write_fd);
		}
		if (client_[client_fd].cgi_pid != -1) {
			kill(client_[client_fd].cgi_pid, SIGKILL);
			waitpid(client_[client_fd].cgi_pid, NULL, 0);
		}
		client_.erase(client_fd);
	}
}

std::vector<int> Client::HandleCgiTimeout() {
	std::vector<int> timed_out_fds;
	std::time_t now = std::time(NULL);
	for (std::map<int, ClientData>::iterator it = client_.begin(); it != client_.end(); ++it) {
		if (it->second.is_waiting_cgi && now - it->second.cgi_start_time > 10) {
			HttpResponse err;
			err.setStatusCode(504);
			it->second.write_buffer.append(err.serialize());
			timed_out_fds.push_back(it->first);
			if (it->second.cgi_read_fd != -1) {
				pipe_to_client_.erase(it->second.cgi_read_fd);
				it->second.cgi_read_fd = -1;
			}
			if (it->second.cgi_write_fd != -1) {
				pipe_to_client_.erase(it->second.cgi_write_fd);
				it->second.cgi_write_fd = -1;
			}
			if (it->second.cgi_pid != -1) {
				kill(it->second.cgi_pid, SIGKILL);
				waitpid(it->second.cgi_pid, NULL, 0);
				it->second.cgi_pid = -1;
			}
			it->second.is_waiting_cgi = false;
		}
	}
	return timed_out_fds;
}
