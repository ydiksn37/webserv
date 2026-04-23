#include <iostream>
#include <sstream>
#include <string>
#include <sys/epoll.h>

std::string Engine(const std::string& request)
{
	(void) request;

	// std::string body = 
	// 	"HTTP/1.1 200 OK\r\n"
	// 	"Content-Type: text/html\r\n"
	// 	"Content-Length: 47\r\n"
	// 	"Connection: keep-alive\r\n"
	// 	"\r\n"
	// 	"<html><body><h1>Hello World!</h1></body></html>";
	//
	std::string body = 
		"<html>\n"
		"<head><meta charset=\"UTF-8\"></head>\n"
		"<body>\n"
		"  <h1>POST Test Form</h1>\n"
		"  <p>下のボタンを押すと、このサーバーにPOSTリクエストが飛びます。</p>\n"
		"  \n"
		"  <form action=\"/\" method=\"POST\">\n"
		"    <label>お名前: </label>\n"
		"    <input type=\"text\" name=\"username\" value=\"Taro\"><br><br>\n"
		"    <label>メッセージ: </label>\n"
		"    <input type=\"text\" name=\"message\" value=\"Hello Server!\"><br><br>\n"
		"    <button type=\"submit\">POST送信！</button>\n"
		"  </form>\n"
		"</body>\n"
		"</html>";

	std::ostringstream response_stream;
	response_stream << "HTTP/1.1 200 OK\r\n"
		<< "Content-Type: text/html\r\n"
		<< "Content-Length: " << body.length() << "\r\n"
		<< "Connection: keep-alive\r\n"
		<< "\r\n"
		<< body;

	std::string response = response_stream.str();
	return response;
}

std::string get_epoll_events_str(uint32_t events) {
	std::string str = "";

	if (events & EPOLLIN)      str += "EPOLLIN ";
	if (events & EPOLLOUT)     str += "EPOLLOUT ";

#ifdef EPOLLRDHUP
	if (events & EPOLLRDHUP)   str += "EPOLLRDHUP ";
#endif

	if (events & EPOLLERR)     str += "EPOLLERR ";    // エラー発生
	if (events & EPOLLHUP)     str += "EPOLLHUP ";    // ハングアップ（異常切断）
	if (events & EPOLLPRI)     str += "EPOLLPRI ";    // 緊急データ(OOB)到着

	if (events & EPOLLET)      str += "EPOLLET ";     // エッジトリガーモード
	if (events & EPOLLONESHOT) str += "EPOLLONESHOT "; // ワンショットモード

	if (str.empty())           str = "UNKNOWN ";

	return str;
}

int GetContentLength(std::string header)
{
	std::string line;
	std::stringstream ss_head(header);
	while(std::getline(ss_head,line))
	{
		std::stringstream ss(line);
		std::string param;
		ss>>param;
		if(param == "Content-Length:")
		{
			int length;
			ss>>length;
			return length;
		}
	}
	return 0;
}
