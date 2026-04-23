#include "Client.hpp"
#include <unistd.h>
#include "eventloop_int.hpp"

bool endswith(std::string str, std::string suffix)
{
	if(str.size() < suffix.size())
		return false;
	return str.substr(str.size()-suffix.size()) == suffix;
}

Client::Client(){}

int Client::Read(int fd)
{
	client_[fd].is_read = 1;
	char tmp_buffer[buffer_size];
	int read_size = read(fd, tmp_buffer, buffer_size - 1);
	if(read_size <= 0)
	{
		client_.erase(fd);
		return read_size;
	}
	tmp_buffer[read_size] = '\0';
	client_[fd].buffer.append(tmp_buffer);

	if(endswith(client_[fd].buffer, "\r\n\r\n")) // TODO bodyがある場合にも対応
	{
		// GetContentLength(client_[fd].buffer);
		client_[fd].is_read = 0;
		client_[fd].buffer = Engine(client_[fd].buffer);
	}
	return read_size;
}

void Client::Write(int fd)
{
	client_[fd].is_read = 0;
	int write_size = write(fd, client_[fd].buffer.c_str(), client_[fd].buffer.size());
	client_[fd].buffer.erase(0,write_size);

	if(client_[fd].buffer.empty())
		client_.erase(fd);
}

bool Client::IsRead(int fd)
{
	return client_[fd].is_read;
}
