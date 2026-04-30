#include "Epoll.hpp"
#include "EventLoop.hpp"
#include <iostream>

int main()
{
	try {
		EventLoop();
	} catch (const std::exception& e) {
		std::cout<<e.what()<<std::endl;
		return 1;
	}
}
