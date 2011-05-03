#ifndef PROTOCOL_H
#define PROTOCOL_H
#include "main.h"
#include "socket.h"

class Base;
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
		std::vector<char> channelTypes();
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
		virtual void quitServer(std::string reason = "");
		virtual void kickUser(std::string channel, std::string user, std::string reason = "");
		virtual void changeNick(std::string newNick);
		
		virtual void sendNumeric(std::string numeric, std::string target, std::vector<std::string> numericData);
		virtual void killUser(std::string user, std::string reason);
		virtual void setXLine(char lineType, std::string hostmask, std::string time, std::string reason);
		virtual void removeXLine(char lineType, std::string hostmask);
		
		virtual void sendOther(std::string rawLine);
		
		virtual void addClient(std::string nick, std::string ident, std::string host, std::string gecos);
		virtual std::list<std::string> clients();
		virtual std::tr1::unordered_map<std::string, std::string> clientInfo(std::string client);
		virtual std::list<std::string> userModes(std::string client);
	protected:
		std::string serverName;
		bool keepServer;
		unsigned short debugLevel;
		std::tr1::unordered_map<std::string, std::string> serverConf;
		Socket* connection;
		Base* botBase;
};
#include "base.h"
#endif