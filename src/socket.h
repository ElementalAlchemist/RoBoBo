#pragma once
#include "main.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

class Socket {
	public:
		Socket();
		virtual ~Socket();
		virtual unsigned int apiVersion() = 0;
		virtual void connectServer(std::string server, std::string port, std::string bindAddr = "");
		virtual std::string receive();
		virtual void sendData(std::string line);
		virtual void closeConnection();
		bool isConnected();
	protected:
		int socketfd;
		bool connected;
};