#pragma once
#include "main.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

enum SocketConnectStatus { SOCKCONN_SUCCESS = 0x0, SOCKCONN_HOST = 0x1, SOCKCONN_FD = 0x2, SOCKCONN_BIND = 0x4, SOCKCONN_CONNECT = 0x8 };

class Socket {
	public:
		Socket();
		virtual ~Socket();
		virtual unsigned int apiVersion() = 0;
		virtual unsigned int connectServer(std::string server, std::string port, std::string bindAddr = "") { return SOCKCONN_FD; } // default to an error with socket() for an undefined function
		virtual std::string receive() { return ""; }
		virtual void sendData(std::string line) {}
		virtual void closeConnection() {}
		bool isConnected() { return connected; }
	protected:
		int socketfd;
		bool connected;
};