#ifndef PROTOCOL_H
#define PROTOCOL_H
#include "main.h"
#include "socket.h"

class Protocol {
	public:
		Protocol(std::string serverAddress, std::tr1::unordered_map<std::string, std::string> confVars, Base* theBase, unsigned short debug);
		virtual ~Protocol();
		virtual unsigned int apiVersion() = 0;
		virtual void connectServer();
		bool stillConnected();
		bool shouldReset();
		virtual bool isClient();
		std::tr1::unordered_map<std::string, std::string> info();
		std::list<std::pair<char, char> > prefixes();
		std::vector<std::vector<char> > channelModes();
		std::list<std::string> channels();
		std::string channelTopic(std::string channel);
		std::list<std::string> channelUsers(std::string channel);
		std::string userIdent(std::string user);
		std::string userHost(std::string user);
		std::pair<char, char> userStatus(std::string channel, std::string user);
		
		virtual void sendMsg(std::string target, std::string message);
		virtual void sendNotice(std::string target, std::string message);
		virtual void setMode(std::string target, std::string mode);
		virtual void removeMode(std::string target, std::string mode);
		virtual void joinChannel(std::string channel, std::string key = "");
		virtual void partChannel(std::string channel, std::string reason = "");
		virtual void quitServer();
		virtual void kickUser(std::string channel, std::string user, std::string reason);
		
		virtual void sendNumeric(std::string numeric, std::string target, std::vector<std::string> numericData);
		virtual void killUser(std::string user, std::string reason);
		virtual void setXLine(char lineType, std::string hostmask, std::string time, std::string reason);
		virtual void removeXLine(char lineType, std::string hostmask);
		
		virtual void sendOther(std::string rawLine);
	protected:
		std::string serverName;
		unsigned short debugLevel;
		std::tr1::unordered_map<std::string, std::string> serverConf;
	private:
		Base* botBase;
		Socket* connection;
};
#endif