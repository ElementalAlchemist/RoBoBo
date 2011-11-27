#ifndef MODULES_H // See the MODULES doc file for more information on utilizing this class.
#define MODULES_H
#include "main.h"
#include "config.h"
#include <dlfcn.h>
#include <map>

#define MODULE_SPAWN(name) extern "C" Module* spawn(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug) {\
	return new name (modConf, modFace, modName, dir, debug); \
}

enum Priority { PRI_HIGH, PRI_MEDIUM_HIGH, PRI_NORMAL, PRI_MEDIUM_LOW, PRI_LOW };

class Base;
class Module {
	public:
		Module(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug);
		virtual ~Module();
		virtual int botAPIversion() = 0;
		void reconf(std::tr1::unordered_map<std::string, std::string> modConf);
		virtual Priority receivePriority();
		virtual bool onLoadComplete();
		virtual void onRehash();
		virtual void onModuleChange();
		virtual bool onChannelMsg(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message);
		virtual bool onUserMsg(std::string server, std::string client, std::string nick, std::string message);
		virtual bool onChannelNotice(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message);
		virtual bool onUserNotice(std::string server, std::string client, std::string nick, std::string message);
		virtual bool onChannelCTCP(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message);
		virtual bool onUserCTCP(std::string server, std::string client, std::string nick, std::string message);
		virtual bool onChannelCTCPReply(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message);
		virtual bool onUserCTCPReply(std::string server, std::string client, std::string nick, std::string message);
		virtual void onChannelJoinPre(std::string server, std::string channel, std::string hostmask);
		virtual void onChannelJoinPost(std::string server, std::string channel, std::string hostmask);
		virtual void onChannelPartPre(std::string server, std::string channel, std::string hostmask, std::string reason);
		virtual void onChannelPartPost(std::string server, std::string channel, std::string hostmask, std::string reason);
		virtual void onUserConnectPre(std::string server, std::string nick, std::string ident, std::string host, std::string gecos);
		virtual void onUserConnectPost(std::string server, std::string nick, std::string ident, std::string host, std::string gecos);
		virtual void onUserQuitPre(std::string server, std::string hostmask, std::string reason);
		virtual void onUserQuitPost(std::string server, std::string hostmask, std::string reason);
		virtual void onNickChangePre(std::string server, std::string oldNick, std::string newNick);
		virtual void onNickChangePost(std::string server, std::string oldNick, std::string newNick);
		virtual void onChannelKickPre(std::string server, std::string channel, std::string kicker, std::string kickee, std::string reason);
		virtual void onChannelKickPost(std::string server, std::string channel, std::string kicker, std::string kickee, std::string reason);
		virtual void onChannelModePre(std::string server, std::string channel, std::string setter, std::string mode, bool add, std::string param);
		virtual void onChannelModePost(std::string server, std::string channel, std::string setter, std::string mode, bool add, std::string param);
		virtual void onUserModePre(std::string server, std::string client, std::string mode, bool add);
		virtual void onUserModePost(std::string server, std::string client, std::string mode, bool add);
		virtual void onUserOperPre(std::string server, std::string user, std::string opertype);
		virtual void onUserOperPost(std::string server, std::string user, std::string opertype);
		virtual void onNumeric(std::string server, std::string client, std::string numeric, std::vector<std::string> parsedLine);
		virtual void onOtherData(std::string server, std::string client, std::vector<std::string> parsedLine);
		virtual void onPreConnect(std::string server);
		virtual void onConnect(std::string server, std::string client);
		virtual void onQuit(std::string server, std::string client);
		
		virtual std::string onChannelMessageOut(std::string server, std::string client, std::string target, char status, std::string message);
		virtual void onChannelMessageSend(std::string server, std::string client, std::string target, char status, std::string message);
		virtual std::string onUserMessageOut(std::string server, std::string client, std::string target, std::string message);
		virtual void onUserMessageSend(std::string server, std::string client, std::string target, std::string message);
		virtual std::string onChannelNoticeOut(std::string server, std::string client, std::string target, char status, std::string message);
		virtual void onChannelNoticeSend(std::string server, std::string client, std::string target, char status, std::string message);
		virtual std::string onUserNoticeOut(std::string server, std::string client, std::string target, std::string message);
		virtual void onUserNoticeSend(std::string server, std::string client, std::string target, std::string message);
		virtual std::string onChannelCTCPOut(std::string server, std::string client, std::string target, char status, std::string message);
		virtual void onChannelCTCPSend(std::string server, std::string client, std::string target, char status, std::string message);
		virtual std::string onUserCTCPOut(std::string server, std::string client, std::string target, std::string message);
		virtual void onUserCTCPSend(std::string server, std::string client, std::string target, std::string message);
		virtual std::string onChannelCTCPReplyOut(std::string server, std::string client, std::string target, char status, std::string message);
		virtual void onChannelCTCPReplySend(std::string server, std::string client, std::string target, char status, std::string message);
		virtual std::string onUserCTCPReplyOut(std::string server, std::string client, std::string target, std::string message);
		virtual void onUserCTCPReplySend(std::string server, std::string client, std::string target, std::string message);
		
		virtual std::string description();
		virtual std::vector<std::string> abilities();
		virtual std::vector<std::string> supports();
	protected:
		std::string moduleName, baseDir;
		unsigned short debugLevel;
		std::tr1::unordered_map<std::string, std::string> config;
		void sendPrivMsg(std::string server, std::string client, std::string target, std::string message);
		void sendNotice(std::string server, std::string client, std::string target, std::string message);
		void sendCTCP(std::string server, std::string client, std::string target, std::string type, std::string params = "");
		void sendCTCPReply(std::string server, std::string client, std::string target, std::string type, std::string data = "");
		void joinChannel(std::string server, std::string client, std::string channel, std::string key = "");
		void partChannel(std::string server, std::string client, std::string channel, std::string reason = "");
		void kickChannelUser(std::string server, std::string client, std::string channel, std::string nick, std::string reason = "");
		void setMode(std::string server, std::string client, std::string channel, std::list<std::string> addModes, std::list<std::string> remModes);
		void changeNick(std::string server, std::string client, std::string nick);
		void oper(std::string server, std::string client, std::string username = "", std::string password = "");
		void killUser(std::string server, std::string client, std::string user, std::string reason);
		void setXLine(std::string server, std::string client, std::string lineType, std::string hostmask, time_t duration, std::string reason);
		void removeXLine(std::string server, std::string client, std::string lineType, std::string hostmask);
		std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, time_t> > listXLines(std::string server);
		void sendSNotice(std::string server, char snomask, std::string text);
		void sendOtherCommand(std::string server, std::string command, std::string data); // catch-all, just in case
		std::string addClient(std::string server, std::string nick, std::string ident, std::string host, std::string gecos);
		void removeClient(std::string server, std::string client, std::string reason = "");
		std::set<std::string> clients(std::string server);
		std::tr1::unordered_map<std::string, std::string> clientInfo(std::string server, std::string client);
		bool connectServer(std::string server);
		void quitServer(std::string server, std::string reason = "");
		bool loadModule(std::string modName);
		void unloadModule();
		void unloadModule(std::string modName);
		void rehashBot();
		unsigned int botVersion();
		std::list<std::string> servers();
		std::tr1::unordered_map<std::string, Module*> modules();
		std::multimap<std::string, std::string> modAbilities();
		std::tr1::unordered_map<std::string, std::vector<std::string> > modSupports();
		std::vector<std::string> splitHostmask(std::string hostmask);
		std::vector<std::string> splitBySpace(std::string line);
		std::string stripColors(std::string message);
		bool serverIsClient(std::string server);
		std::tr1::unordered_map<std::string, std::string> serverInfo(std::string server);
		std::vector<std::vector<std::string> > serverChanModes(std::string server);
		std::list<std::pair<std::string, char> > serverPrefixes(std::string server);
		std::string compareStatus(std::string server, std::set<std::string> statuses);
		std::list<std::string> channels(std::string server);
		std::string channelTopic(std::string server, std::string channel);
		std::set<std::string> channelUsers(std::string server, std::string channel);
		std::set<std::string> channelModes(std::string server, std::string channel);
		std::string userIdent(std::string server, std::string user);
		std::string userHost(std::string server, std::string user);
		std::list<std::string> userModes(std::string server, std::string user);
		std::pair<std::string, char> userStatus(std::string server, std::string channel, std::string user);
		std::string userMetadata(std::string server, std::string user, std::string key);
	private:
		Base* serverData;
};
#include "protocol.h"
#endif