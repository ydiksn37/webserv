#include <string>
#include <sys/epoll.h>

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
