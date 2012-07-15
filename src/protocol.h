#pragma once
#include "main.h"

#include "socket.h"

class Base;

class Protocol {
	public:
		Protocol(std::string server, std::map<std::string, std::string> conf, std::string workDir, bool dumpLogs, unsigned short debug, Base* botptr);
		virtual ~Protocol();
		virtual unsigned int apiVersion() = 0;
		virtual void connectServer();
		virtual void disconnectServer(std::string reason);
		virtual bool isConnected();
		virtual bool deadServer();
		virtual bool isClient();
		void endDebug();
	protected:
		std::string serverName, workingDir;
		std::map<std::string, std::string> config;
		bool log;
		unsigned short debugLevel;
		std::shared_ptr<Socket> assignSocket(std::string socketType);
	private:
		Base* bot;
};

#include "base.h"