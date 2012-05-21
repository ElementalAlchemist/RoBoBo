#pragma once
#include "main.h"
#include <dlfcn.h>

enum LoadResult { LOAD_SUCCESS, LOAD_ALREADYLOADED, LOAD_ERROR, LOAD_INCOMPATIBLE, LOAD_NODEPENDS, LOAD_FAILURE };

#include "socket.h"
#include "protocol.h"
#include "module.h"

typedef void* (module_spawn_t)(std::string, std::map<std::string, std::string>, std::string, unsigned short, Base*);

class Base {
	public:
		Base(std::string working, std::string config, std::string configFileName, unsigned short debug, bool log);
		void readConfiguration();
		void loadModules();
		bool completeStartupLoad(std::string modName, Module* modptr);
		void connectServers();
		void checkServers();
		void unloadEverything();
		LoadResult loadModule(std::string modName);
		void unloadModule(std::string modName, bool wasLoaded);
		void connectServer(std::string server);
		void disconnectServer(std::string server);
		Socket* loadSocket(std::string sockettype);
		void unloadSocket(std::string sockettype, Socket* socketptr);
		void modChanMsgHook(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message);
		void modUserMsgHook(std::string server, std::string client, std::string nick, std::string message);
		void modChanNoticeHook(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message);
		void modUserNoticeHook(std::string server, std::string client, std::string nick, std::string message);
		void modChanCTCPHook(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string data);
		void modUserCTCPHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string data);
		void modChanCTCPReplyHook(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string data);
		void modUserCTCPReplyHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string data);
		void modChanModeHook(std::string server, std::string client, std::string channel, bool add, std::string mode, std::string param);
		void modUserModeHook(std::string server, std::string client, bool add, std::string mode);
		void modUserSNOMaskHook(std::string server, std::string client, bool add, std::string snomask);
		void modChanTopicHook(std::string server, std::string client, std::string channel, std::string topic);
		void modChanJoinHook(std::string server, std::string client, std::string channel, std::string nick);
		void modChanPartHook(std::string server, std::string client, std::string channel, std::string nick, std::string reason);
		void modUserConnectHook(std::string server, std::string nick);
		void modUserQuitHook(std::string server, std::string client, std::string nick, std::string reason);
		void modUserNickHook(std::string server, std::string client, std::string oldNick, std::string newNick);
		void modNumericHook(std::string server, std::string client, std::string numeric, std::vector<std::string> data);
		void modOperHook(std::string server, std::string nick, std::string operType);
		void modSNoticeHook(std::string server, std::string snotype, std::string message);
		void modUserMetadataHook(std::string server, std::string nick, std::string dataKey, std::string dataValue);
		void modServerDataHook(std::string server, std::string dataType, std::string params);
		void modXLineAddHook(std::string server, std::string lineType, std::string mask, std::string setter, time_t expiry, std::string reason);
		void modXLineRemoveHook(std::string server, std::string lineType, std::string mask);
		void modServerConnectHook(std::string server, std::string newServerName);
		void modServerQuitHook(std::string server, std::string quitServerName, std::string reason);
		void modOtherDataHook(std::string server, std::string client, std::vector<std::string> lineTokens);
		void modChanMsgOutHook(std::string server, std::string client, std::string channel, char status, std::string &message);
		void modChanMsgSendHook(std::string server, std::string client, std::string channel, char status, std::string message);
		void modUserMsgOutHook(std::string server, std::string client, std::string channel, char status, std::string &message);
		void modUserMsgSendHook(std::string server, std::string client, std::string channel, char status, std::string message);
		void modChanNoticeOutHook(std::string server, std::string client, std::string channel, char status, std::string &message);
		void modChanNoticeSendHook(std::string server, std::string client, std::string channel, char status, std::string message);
		void modUserNoticeOutHook(std::string server, std::string client, std::string nick, std::string &message);
		void modUserNoticeSendHook(std::string server, std::string client, std::string nick, std::string message);
		void modChanCTCPOutHook(std::string server, std::string client, std::string channel, char status, std::string &ctcp, std::string &params);
		void modChanCTCPSendHook(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params);
		void modUserCTCPOutHook(std::string server, std::string client, std::string nick, std::string &ctcp, std::string &params);
		void modUserCTCPSendHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
		void modChanCTCPReplyOutHook(std::string server, std::string client, std::string channel, char status, std::string &ctcp, std::string &params);
		void modChanCTCPReplySendHook(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params);
		void modUserCTCPReplyOutHook(std::string server, std::string client, std::string nick, std::string &ctcp, std::string &params);
		void modUserCTCPReplySendHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
		// TODO: Protocol hooks
	private:
		const std::string workingDir, configDir, configName;
		const unsigned short debugLevel;
		const bool logDump;
		bool startup;
		std::unordered_map<std::string, std::unordered_map<std::string, std::string>> serverConfig, moduleConfig;
		std::list<std::string> startupServers, startupModules;
		std::map<std::string, Protocol*> servers;
		std::map<std::string, Module*> highModules, mediumHighModules, normalModules, mediumLowModules, lowModules;
		std::unordered_map<std::string, void*> moduleFiles;
		std::map<std::string, Priority> modulePriority;
		std::map<std::string, std::list<std::string>> moduleServices, moduleSupports;
		std::map<std::string, std::string> moduleDescriptions;
		std::unordered_map<std::string, void*> socketFiles;
		std::mutex modHookMutex;
};