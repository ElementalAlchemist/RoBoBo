#ifndef PROTOCOL_H // See the PROTOCOL doc file for more information on utilizing this class.
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
		virtual std::list<std::pair<std::string, char> > prefixes();
		virtual std::set<char> channelTypes();
		virtual std::vector<std::vector<std::string> > channelModes();
		virtual std::list<std::string> channels();
		virtual std::string channelTopic(std::string channel);
		virtual std::set<std::string> channelUsers(std::string channel);
		virtual std::string userIdent(std::string user);
		virtual std::string userHost(std::string user);
		virtual std::pair<std::string, char> userStatus(std::string channel, std::string user);
		virtual std::string compareStatus(std::set<std::string> statuses);
		
		virtual void sendMsg(std::string client, std::string target, std::string message);
		virtual void sendNotice(std::string client, std::string target, std::string message);
		virtual void setMode(std::string client, std::string target, std::string mode);
		virtual void removeMode(std::string client, std::string target, std::string mode);
		virtual void joinChannel(std::string client, std::string channel, std::string key = "");
		virtual void partChannel(std::string client, std::string channel, std::string reason = "");
		virtual void quitServer(std::string reason = "");
		virtual void kickUser(std::string client, std::string channel, std::string user, std::string reason = "");
		virtual void changeNick(std::string client, std::string newNick);
		virtual void oper(std::string client, std::string username, std::string password);
		
		virtual void killUser(std::string client, std::string user, std::string reason);
		virtual void setXLine(std::string client, char lineType, std::string hostmask, time_t duration, std::string reason);
		virtual void removeXLine(std::string client, char lineType, std::string hostmask);
		virtual std::tr1::unordered_map<char, std::tr1::unordered_map<std::string, time_t> > listXLines(); // unordered_map { line type => unordered_map { hostmask => time to expiry } }
		virtual void sendSNotice(char snomask, std::string text);
		
		virtual void sendOther(std::string rawLine);
		
		virtual void addClient(std::string nick, std::string ident, std::string host, std::string gecos);
		virtual void removeClient(std::string client, std::string reason = "");
		virtual std::set<std::string> clients();
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