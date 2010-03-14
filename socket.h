#ifndef SOCKET_CPP
#define SOCKET_CPP
#include "main.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <cstdio>

class Socket {
	public:
		Socket();
		~Socket();
		void connectServer(std::string address, unsigned short port);
		bool sendData(std::string message);
		std::string receive();
		void closeConnection();
		bool isConnected();
	private:
		int socketfd;
		sockaddr_in socketAddr;
		bool connected;
};

#endif
