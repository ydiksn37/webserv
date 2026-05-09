#ifndef EVENTLOOP_HPP
# define EVENTLOOP_HPP

# include "Config.hpp"

void EventLoop(const Config& config);

class EventLoopException : public std::runtime_error {
	public:
		EventLoopException(const std::string& msg) : std::runtime_error(msg) {}
};

#endif
