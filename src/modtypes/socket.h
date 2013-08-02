#pragma once
#include "../main.h"

class Socket {
	public:
		virtual ~Socket() {}
		virtual unsigned int apiVersion() = 0;
		virtual void connect(const std::string& server, const std::string& port, const std::string& bindAddr = "") {}
		virtual std::string receive() { return ""; }
		virtual void send(const std::string& data) {}
		virtual void closeConnection() {}
		bool isConnected() { return connected; }
	protected:
		bool connected;
};