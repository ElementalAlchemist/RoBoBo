#pragma once
#include "../main.h"

class Socket {
	public:
		Socket() : delimiter("\r\n") {}
		virtual ~Socket() {}
		virtual unsigned int apiVersion() = 0;
		void setDelimiter(const std::string& delim) { delimiter = delim; }
		virtual void connectServer(const std::string& server, const std::string& port, const std::string& bindAddr = "") {}
		virtual std::string receive() { return ""; }
		virtual void send(const std::string& data) {}
		virtual void closeConnection() {}
		bool isConnected() { return connected; }
	protected:
		bool connected;
		std::string delimiter;
};