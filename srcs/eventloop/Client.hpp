#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <map>
#include <string>


class Client
{
	public:
		Client();
		int Read(int fd);
		void Write(int fd);
		bool IsReadEnd(int fd);
		bool IsWriteEnd(int fd);
		bool IsRead(int fd);
	private:
		struct ClientData
		{
			std::string buffer;
			bool is_read;
			ClientData():is_read(true){}
		};
		std::map<int,ClientData> client_;
};

#endif
