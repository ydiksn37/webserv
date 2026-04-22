#include <string>

std::string Engine(const std::string& request)
{
	(void) request;

	std::string response = 
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: 47\r\n"
		"Connection: keep-alive\r\n"
		"\r\n"
		"<html><body><h1>Hello World!</h1></body></html>";
	return response;
}
