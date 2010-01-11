#include "main.h"
#include "config.cpp"
#include <dlfcn.h>

#ifndef MODULES_H
#define MODULES_H
class Module {
	public:
		Module();
		virtual ~Module();
		void init(std::tr1::unordered_map<std::string, std::string> modConf, ModuleInterface* modFace);
		virtual void onLoadComplete();
		virtual void onChannelMsg(std::string server, std::string channel, char target, std::string nick, std::string message);
		virtual void onUserMsg(std::string server, std::string nick, std::string message);
		virtual void onChannelNotice(std::string server, std::string channel, char target, std::string nick, std::string message);
		virtual void onUserNotice(std::string server, std::string nick, std::string message);
		virtual void onChannelCTCP(std::string server, std::string channel, char target, std::string nick, std::string message);
		virtual void onUserCTCP(std::string server, std::string nick, std::string message);
		virtual void onChannelCTCPReply(std::string server, std::string channel, char target, std::string nick, std::string message);
		virtual void onUserCTCPReply(std::string server, std::string nick, std::string message);
		virtual void onChannelJoin(std::string server, std::string channel, std::string hostmask);
		virtual void onChannelPart(std::string server, std::string channel, std::string hostmask, std::string reason);
		virtual void onUserQuit(std::string server, std::string hostmask, std::string reason);
		virtual void onChannelKick(std::string server, std::string channel, std::string kicker, std::string kickee, std::string reason);
		virtual void onChannelMode(std::string server, std::string channel, std::string setter, char mode, bool add, std::string param);
		virtual void onNumeric(std::string server, std::string numeric, std::vector<std::string> parsedLine);
		virtual void onOtherData(std::string server, std::vector<std::string> parsedLine);
		
		virtual void onOutChannelMessage(std::string server, std::string target, char status, std::string message);
		virtual void onOutUserMessage(std::string server, std::string target, std::string message);
		virtual void onOutChannelNotice(std::string server, std::string target, char status, std::string message);
		virtual void onOutUserNotice(std::string server, std::string target, std::string message);
		virtual void onOutChannelCTCP(std::string server, std::string target, char status, std::string message);
		virtual void onOutUserCTCP(std::string server, std::string target, std::string message);
		virtual void onOutChannelCTCPReply(std::string server, std::string target, char status, std::string message);
		virtual void onOutUserCTCPReply(std::string server, std::string target, std::string message);
		
		virtual std::string getDesc();
		virtual std::string getHelp();
		virtual std::vector<std::string> getRequiredVars();
	protected:
		std::string moduleName;
		std::tr1::unordered_map<std::string, std::string> config;
		void sendPrivMsg(std::string server, std::string target, std::string message);
		void sendNotice(std::string server, std::string target, std::string message);
		void sendCTCP(std::string server, std::string target, std::string type, std::string params = "");
		void sendCTCPReply(std::string server, std::string target, std::string type, std::string data = "");
		void joinChannel(std::string server, std::string channel, std::string key = "");
		void partChannel(std::string server, std::string channel, std::string reason);
		void kickChannelUser(std::string server, std::string channel, std::string nick, std::string reason);
		void setMode(std::string server, std::string channel, char mode, bool add, std::string param = "");
		void quitServer(std::string server, std::string reason);
		std::list<std::string> getServers();
		std::tr1::unordered_map<std::string, Module*> getModules();
		std::vector<std::string> splitHostmask(std::string hostmask);
		std::vector<std::string> splitBySpace(std::string line);
		std::vector<std::vector<char> > getServerChanModes(std::string server);
		std::tr1::unordered_map<char, char> getServerPrefixes(std::string server);
	private:
		ModuleInterface* serverData;
};

class ModuleInterface {
	public:
		ModuleInterface(std::string confdir, std::string confname, unsigned short debug);
		void sendToServer(std::string server, std::string rawLine);
		std::tr1::unordered_map<std::string, std::string> getServerData(std::string server);
		std::vector<std::vector<char> > getServerChanModes(std::string server);
		std::tr1::unordered_map<char, char> getServerPrefixes(std::string server);
		void callHook(std::string server, std::vector<std::string> parsedLine);
		void callHookOut(std::string server, std::vector<std::string> parsedLine);
		std::tr1::unordered_map<std::string, Module*> getModules();
		std::list<std::string> getServers();
		std::list<std::string> getChannels(std::string server);
		std::string getChannelTopic(std::string server, std::string channel);
		std::list<std::string> getChannelUsers(std::string server, std::string channel);
		std::pair<char, char> getUserStatus(std::string server, std::string channel, std::string user);
		void connectServer(std::string serverName, std::tr1::unordered_map<std::string, std::string> serverConf);
		void loadModule(std::string modName, std::tr1::unordered_map<std::string, std::string> modConf);
		void removeServer(std::string server);
	private:
		std::tr1::unordered_map<std::string, Server*> servers;
		std::tr1::unordered_map<std::string, Module*> modules;
		unsigned short debugLevel;
		std::string parseNickFromHost(std::string host);
		bool charIsNumeric(char number);
		bool isChanType(char chanPrefix);
};
#endif