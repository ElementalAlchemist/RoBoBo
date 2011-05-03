#ifndef MODULES_H
#define MODULES_H
#include "main.h"
#include "config.h"
#include <dlfcn.h>
#include <map>

enum Priority { HIGH, MEDIUM_HIGH, NORMAL, MEDIUM_LOW, LOW };

class Base;
class Module {
	public:
		Module(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, unsigned short debug);
		virtual ~Module();
		virtual int botAPIversion() = 0;
		void reconf(std::tr1::unordered_map<std::string, std::string> modConf);
		virtual Priority receivePriority();
		virtual bool onLoadComplete();
		virtual void onRehash();
		virtual void onModuleChange();
		virtual void onChannelMsg(std::string server, std::string channel, char target, std::string nick, std::string message);
		virtual void onUserMsg(std::string server, std::string nick, std::string message);
		virtual void onChannelNotice(std::string server, std::string channel, char target, std::string nick, std::string message);
		virtual void onUserNotice(std::string server, std::string nick, std::string message);
		virtual void onChannelAction(std::string server, std::string channel, char target, std::string nick, std::string message);
		virtual void onUserAction(std::string server, std::string nick, std::string message);
		virtual void onChannelCTCP(std::string server, std::string channel, char target, std::string nick, std::string message);
		virtual void onUserCTCP(std::string server, std::string nick, std::string message);
		virtual void onChannelCTCPReply(std::string server, std::string channel, char target, std::string nick, std::string message);
		virtual void onUserCTCPReply(std::string server, std::string nick, std::string message);
		virtual void onChannelJoinPre(std::string server, std::string channel, std::string hostmask);
		virtual void onChannelJoinPost(std::string server, std::string channel, std::string hostmask);
		virtual void onChannelPartPre(std::string server, std::string channel, std::string hostmask, std::string reason);
		virtual void onChannelPartPost(std::string server, std::string channel, std::string hostmask, std::string reason);
		virtual void onUserQuitPre(std::string server, std::string hostmask, std::string reason);
		virtual void onUserQuitPost(std::string server, std::string hostmask, std::string reason);
		virtual void onNickChangePre(std::string server, std::string oldNick, std::string newNick);
		virtual void onNickChangePost(std::string server, std::string oldNick, std::string newNick);
		virtual void onChannelKickPre(std::string server, std::string channel, std::string kicker, std::string kickee, std::string reason);
		virtual void onChannelKickPost(std::string server, std::string channel, std::string kicker, std::string kickee, std::string reason);
		virtual void onChannelModePre(std::string server, std::string channel, std::string setter, char mode, bool add, std::string param);
		virtual void onChannelModePost(std::string server, std::string channel, std::string setter, char mode, bool add, std::string param);
		virtual void onNumeric(std::string server, std::string numeric, std::vector<std::string> parsedLine);
		virtual void onOtherData(std::string server, std::vector<std::string> parsedLine);
		virtual void onPreConnect(std::string server);
		virtual void onConnect(std::string server);
		virtual void onQuit(std::string server);
		
		virtual std::string onOutChannelMessage(std::string server, std::string target, char status, std::string message);
		virtual void onSendChannelMessage(std::string server, std::string target, char status, std::string message);
		virtual std::string onOutUserMessage(std::string server, std::string target, std::string message);
		virtual void onSendUserMessage(std::string server, std::string target, std::string message);
		virtual std::string onOutChannelNotice(std::string server, std::string target, char status, std::string message);
		virtual void onSendChannelNotice(std::string server, std::string target, char status, std::string message);
		virtual std::string onOutUserNotice(std::string server, std::string target, std::string message);
		virtual void onSendUserNotice(std::string server, std::string target, std::string message);
		virtual std::string onOutChannelAction(std::string server, std::string target, char status, std::string message);
		virtual void onSendChannelAction(std::string server, std::string target, char status, std::string message);
		virtual std::string onOutUserAction(std::string server, std::string target, std::string message);
		virtual void onSendUserAction(std::string server, std::string target, std::string message);
		virtual std::string onOutChannelCTCP(std::string server, std::string target, char status, std::string message);
		virtual void onSendChannelCTCP(std::string server, std::string target, char status, std::string message);
		virtual std::string onOutUserCTCP(std::string server, std::string target, std::string message);
		virtual void onSendUserCTCP(std::string server, std::string target, std::string message);
		virtual std::string onOutChannelCTCPReply(std::string server, std::string target, char status, std::string message);
		virtual void onSendChannelCTCPReply(std::string server, std::string target, char status, std::string message);
		virtual std::string onOutUserCTCPReply(std::string server, std::string target, std::string message);
		virtual void onSendUserCTCPReply(std::string server, std::string target, std::string message);
		
		virtual std::string description();
		virtual std::vector<std::string> abilities();
		virtual std::vector<std::string> supports();
	protected:
		std::string moduleName;
		unsigned short debugLevel;
		std::tr1::unordered_map<std::string, std::string> config;
		void sendPrivMsg(std::string server, std::string target, std::string message);
		void sendNotice(std::string server, std::string target, std::string message);
		void sendCTCP(std::string server, std::string target, std::string type, std::string params = "");
		void sendCTCPReply(std::string server, std::string target, std::string type, std::string data = "");
		void joinChannel(std::string server, std::string channel, std::string key = "");
		void partChannel(std::string server, std::string channel, std::string reason = "");
		void kickChannelUser(std::string server, std::string channel, std::string nick, std::string reason = "");
		void setMode(std::string server, std::string channel, std::string mode);
		void removeMode(std::string server, std::string channel, std::string mode);
		void changeNick(std::string server, std::string nick);
		void sendNumeric(std::string server, std::string target, std::string numeric, std::vector<std::string> numericData);
		void killUser(std::string server, std::string user, std::string reason);
		void setXLine(std::string server, char lineType, std::string hostmask, std::string duration, std::string reason);
		void removeXLine(std::string server, char lineType, std::string hostmask);
		void sendOtherCommand(std::string server, std::string command, std::string data); // catch-all, just in case
		bool connectServer(std::string server);
		void quitServer(std::string server, std::string reason = "");
		bool loadModule(std::string modName);
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
		std::vector<std::vector<char> > serverChanModes(std::string server);
		std::list<std::pair<char, char> > serverPrefixes(std::string server);
		std::list<std::string> channels(std::string server);
		std::string channelTopic(std::string server, std::string channel);
		std::list<std::string> channelUsers(std::string server, std::string channel);
		std::string userIdent(std::string server, std::string user);
		std::string userHost(std::string server, std::string user);
		std::pair<char, char> userStatus(std::string server, std::string channel, std::string user);
	private:
		Base* serverData;
};
#include "protocol.h"
#endif