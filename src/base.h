#ifndef BASE_H
#define BASE_H
#include "modules.h"

typedef void* (*module_spawn_t)(std::tr1::unordered_map<std::string, std::string>, Base*, std::string, std::string, unsigned short);
typedef void* (*proto_spawn_t)(std::string, std::tr1::unordered_map<std::string, std::string>, Base*, unsigned short);
typedef void* (*socket_spawn_t)();

class Base {
	public:
		Base(std::string confdir, std::string confname, unsigned short debug);
		std::tr1::unordered_map<std::string, std::string> serverData(std::string server);
		std::vector<std::vector<char> > serverChanModes(std::string server);
		std::list<std::pair<char, char> > serverPrefixes(std::string server);
		bool callChanMsgHook(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message);
		bool callUserMsgHook(std::string server, std::string client, std::string nick, std::string message);
		bool callChanNoticeHook(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message);
		bool callUserNoticeHook(std::string server, std::string client, std::string nick, std::string message);
		bool callChannelCTCPHook(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message);
		bool callUserCTCPHook(std::string server, std::string client, std::string nick, std::string message);
		bool callChannelCTCPReplyHook(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message);
		bool callUserCTCPReplyHook(std::string server, std::string client, std::string nick, std::string message);
		void callChannelJoinPreHook(std::string server, std::string client, std::string channel, std::string hostmask);
		void callChannelJoinPostHook(std::string server, std::string client, std::string channel, std::string hostmask);
		void callChannelPartPreHook(std::string server, std::string client, std::string channel, std::string hostmask, std::string reason);
		void callChannelPartPostHook(std::string server, std::string client, std::string channel, std::string hostmask, std::string reason);
		void callUserQuitPreHook(std::string server, std::string client, std::string hostmask, std::string reason);
		void callUserQuitPostHook(std::string server, std::string client, std::string hostmask, std::string reason);
		void callNickChangePreHook(std::string server, std::string client, std::string oldNick, std::string newNick);
		void callNickChangePostHook(std::string server, std::string client, std::string oldNick, std::string newNick);
		void callChannelKickPreHook(std::string server, std::string client, std::string channel, std::string kicker, std::string kickee, std::string reason);
		void callChannelKickPostHook(std::string server, std::string client, std::string channel, std::string kicker, std::string kickee, std::string reason);
		void callChannelModePreHook(std::string server, std::string client, std::string channel, std::string setter, std::string mode, bool add, std::string param);
		void callChannelModePostHook(std::string server, std::string client, std::string channel, std::string setter, std::string mode, bool add, std::string param);
		void callNumericHook(std::string server, std::string client, std::string numeric, std::vector<std::string> parsedLine);
		void callOtherDataHook(std::string server, std::string client, std::vector<std::string> parsedLine);
		void callPreConnectHook(std::string server, std::string client);
		void callConnectHook(std::string server, std::string client);
		void callQuitHook(std::string server, std::string client);
		
		std::string callChannelMessageOutHook(std::string server, std::string client, std::string target, char status, std::string message);
		void callChannelMessageSendHook(std::string server, std::string client, std::string target, char status, stdd::string message);
		std::string callUserMessageOutHook(std::string server, std::string client, std::string target, std::string message);
		void callUserMessageSendHook(std::string server, std::string client, std::string target, std::string message);
		std::string callChannelNoticeOutHook(std::string server, std::string client, std::string target, char status, std::string message);
		void callChannelNoticeSendHook(std::string server, std::string client, std::string target, char status, std::string message);
		std::string callUserNoticeOutHook(std::string server, std::string client, std::string target, std::string message);
		void callUserNoticeSendHook(std::string server, std::string client, std::string target, std::string message);
		std::string callChannelCTCPOutHook(std::string server, std::string client, std::string target, char status, std::string message);
		void callChannelCTCPSendHook(std::string server, std::string client, std::string target, char status, std::string message);
		std::string callUserCTCPOutHook(std::string server, std::string client, std::string target, std::string message);
		void callUserCTCPSendHook(std::string server, std::string client, std::string target, std::string message);
		std::string callChannelCTCPReplyOutHook(std::string server, std::string client, std::string target, char status, std::string message);
		void callChannelCTCPReplySendHook(std::string server, std::string client, std::string target, char status, std::string message);
		std::string callUserCTCPReplyOutHook(std::string server, std::string client, std::string target, std::string message);
		void callUserCTCPReplySendHook(std::string server, std::string client, std::string target, std::string message);
		/*
		void callPreHook(std::string server, std::vector<std::string> parsedLine);
		void callPostHook(std::string server, std::vector<std::string> parsedLine);
		std::string callHookOut(std::string server, std::vector<std::string> parsedLine);
		void callHookSend(std::string server, std::vector<std::string> parsedLine);
		void callPreConnectHook(std::string server);
		void callConnectHook(std::string server);
		void callQuitHook(std::string server);
		void callPreModulesHook(std::string server, std::vector<std::string> parsedLine, std::tr1::unordered_map<std::string, Module*>* modules);
		bool callPostModulesHook(std::string server, std::vector<std::string> parsedLine, std::tr1::unordered_map<std::string, Module*>* modules);
		std::string callModulesHookOut(std::string server, std::vector<std::string> parsedLine, std::tr1::unordered_map<std::string, Module*>* modules);
		void callModulesHookSend(std::string server, std::vector<std::string> parsedLine, std::tr1::unordered_map<std::string, Module*>* modules);
		void callPreConnectModulesHook(std::string server, std::tr1::unordered_map<std::string, Module*>* modules);
		void callConnectModulesHook(std::string server, std::tr1::unordered_map<std::string, Module*>* modules);
		void callQuitModulesHook(std::string server, std::tr1::unordered_map<std::string, Module*>* modules);
		*/
		void sendPrivMsg(std::string server, std::string client, std::string target, std::string message);
		void sendNotice(std::string server, std::string client, std::string target, std::string message);
		void setMode(std::string server, std::string client, std::string target, std::string mode);
		void removeMode(std::string server, std::string client, std::string target, std::string mode);
		void joinChannel(std::string server, std::string client, std::string channel, std::string key = "");
		void partChannel(std::string server, std::string client, std::string channel, std::string reason = "");
		void quitServer(std::string server, std::string reason = "");
		void kickUser(std::string server, std::string client, std::string channel, std::string user, std::string reason = "");
		void changeNick(std::string server, std::string client, std::string newNick);
		void oper(std::string server, std::string client, std::string username, std::string password);
		void killUser(std::string server, std::string client, std::string user, std::string reason);
		void setXLine(std::string server, std::string client, char lineType, std::string hostmask, time_t duration, std::string reason);
		void removeXLine(std::string server, std::string client, char lineType, std::string hostmask);
		std::tr1::unordered_map<char, std::tr1::unordered_map<std::string, time_t> > listXLines(std::string server);
		void sendSNotice(std::string server, char snomask, std::string text);
		void sendOther(std::string server, std::string rawLine);
		void addClient(std::string server, std::string nick, std::string ident, std::string host, std::string gecos);
		void removeClient(std::string server, std::string client, std::string reason);
		bool isChanType(char chanPrefix, std::string server);
		std::vector<std::string> parseLine(std::string rawLine);
		std::tr1::unordered_map<std::string, Module*> loadedModules();
		std::list<std::string> serverList();
		std::multimap<std::string, std::string> moduleAbilities();
		std::tr1::unordered_map<std::string, std::vector<std::string> > moduleSupports();
		bool serverIsClient(std::string server);
		std::list<std::string> channels(std::string server);
		std::string channelTopic(std::string server, std::string channel);
		std::list<std::string> channelUsers(std::string server, std::string channel);
		std::string userIdent(std::string server, std::string user);
		std::string userHost(std::string server, std::string user);
		std::list<std::string> userModes(
		std::pair<std::string, char> userStatus(std::string server, std::string channel, std::string user);
		Socket* assignSocket(std::string socketType);
		void rehash();
		bool connectServer(std::string serverName);
		bool loadModule(std::string modName, bool startup);
		void unloadModule(std::string modName);
		static void* serverCheck_thread(void* ptr);
		void serverCheck();
	private:
		std::tr1::unordered_map<std::string, Protocol*> servers;
		std::tr1::unordered_map<std::string, Module*> highModules, mediumHighModules, normalModules, mediumLowModules, lowModules;
		std::tr1::unordered_map<std::string, void*> moduleFiles;
		std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > serverConfigs, moduleConfigs;
		std::multimap<std::string, std::string> modAbilities;
		std::tr1::unordered_map<std::string, std::vector<std::string> > modSupports;
		std::tr1::unordered_map<std::string, void*> protocolFiles;
		std::tr1::unordered_map<std::string, void*> socketFiles;
		unsigned short debugLevel;
		std::string directory;
		std::string configName;
		bool charIsNumeric(char number);
		static void* tUnloadMod_thread(void* mip);
		void tUnloadMod();
		std::vector<std::string> moduleToUnload;
		pthread_t serverCheckThread;
		pthread_attr_t detachedState;
};
#endif