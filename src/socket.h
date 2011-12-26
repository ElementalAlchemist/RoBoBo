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
#include <fcntl.h>

class Socket {
	public:
		Socket();
		virtual ~Socket();
		virtual unsigned int apiVersion() = 0;
		virtual bool bindSocket(std::string address);
		virtual void connectServer(std::string address, unsigned short port);
		virtual bool sendData(std::string message);
		virtual std::string receive();
		virtual void closeConnection();
		bool isConnected();
	protected:
		int socketfd;
		sockaddr_in socketAddr;
		bool connected;
};

#endif