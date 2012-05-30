#pragma once
#include "main.h"
#include <dlfcn.h>

enum LoadResult { LOAD_SUCCESS, LOAD_ALREADYLOADED, LOAD_ERROR, LOAD_INCOMPATIBLE, LOAD_NODEPENDS, LOAD_FAILURE };
class Base;

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
		void modMetadataHook(std::string server, std::string target, std::string dataKey, std::string dataValue);
		void modServerDataHook(std::string server, std::string dataType, std::vector<std::string> params);
		void modXLineAddHook(std::string server, std::string lineType, std::string mask, std::string setter, time_t expiry, std::string reason);
		void modXLineRemoveHook(std::string server, std::string lineType, std::string mask);
		void modServerConnectHook(std::string server, std::string newServerName);
		void modServerQuitHook(std::string server, std::string quitServerName, std::string reason);
		void modOtherDataHook(std::string server, std::string client, std::vector<std::string> lineTokens);
		void modChanMsgOutHook(std::string server, std::string client, std::string channel, char status, std::string message);
		void modChanMsgSendHook(std::string server, std::string client, std::string channel, char status, std::string message);
		void modUserMsgOutHook(std::string server, std::string client, std::string nick, std::string message);
		void modUserMsgSendHook(std::string server, std::string client, std::string nick, std::string message);
		void modChanNoticeOutHook(std::string server, std::string client, std::string channel, char status, std::string message);
		void modChanNoticeSendHook(std::string server, std::string client, std::string channel, char status, std::string message);
		void modUserNoticeOutHook(std::string server, std::string client, std::string nick, std::string message);
		void modUserNoticeSendHook(std::string server, std::string client, std::string nick, std::string message);
		void modChanCTCPOutHook(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params);
		void modChanCTCPSendHook(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params);
		void modUserCTCPOutHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
		void modUserCTCPSendHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
		void modChanCTCPReplyOutHook(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params);
		void modChanCTCPReplySendHook(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params);
		void modUserCTCPReplyOutHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
		void modUserCTCPReplySendHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
		
		void sendPrivMsg(std::string server, std::string client, std::string target, std::string message);
		void sendNotice(std::string server, std::string client, std::string target, std::string message);
		void sendCTCP(std::string server, std::string client, std::string target, std::string ctcp, std::string params);
		void sendCTCPReply(std::string server, std::string client, std::string target, std::string ctcp, std::string params);
		void setMode(std::string server, std::string client, std::string target, std::list<std::string> setModes, std::list<std::string> delModes);
		void setSNOMask(std::string server, std::string client, std::string snomask);
		void setChanTopic(std::string server, std::string client, std::string channel, std::string topic);
		void joinChannel(std::string server, std::string client, std::string channel, std::string key);
		void partChannel(std::string server, std::string client, std::string channel, std::string reason);
		std::string addClient(std::string server, std::string nick, std::string ident, std::string host, std::string gecos);
		void removeClient(std::string server, std::string client);
		void changeNick(std::string server, std::string client, std::string newNick);
		void oper(std::string server, std::string client, std::string username, std::string password);
		void sendSNotice(std::string server, std::string snomask, std::string message);
		void setMetadata(std::string server, std::string target, std::string key, std::string value);
		void setXLine(std::string server, std::string client, std::string linetype, std::string mask, time_t duration, std::string reason);
		void delXLine(std::string server, std::string client, std::string linetype, std::string mask);
		void sendOtherData(std::string server, std::string client, std::string line);
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
		void callChanMsgHooks(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message);
		void callUserMsgHooks(std::string server, std::string client, std::string nick, std::string message);
		void callChanNoticeHooks(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message);
		void callUserNoticeHooks(std::string server, std::string client, std::string nick, std::string message);
		void callChanCTCPHooks(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string data);
		void callUserCTCPHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string data);
		void callChanCTCPReplyHooks(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string data);
		void callUserCTCPReplyHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string data);
		void callChanModeHooks(std::string server, std::string client, std::string channel, bool add, std::string mode, std::string param);
		void callUserModeHooks(std::string server, std::string client, bool add, std::string mode);
		void callUserSNOMaskHooks(std::string server, std::string client, bool add, std::string snomask);
		void callChanTopicHooks(std::string server, std::string client, std::string channel, std::string topic);
		void callChanJoinHooks(std::string server, std::string client, std::string channel, std::string nick);
		void callChanPartHooks(std::string server, std::string client, std::string channel, std::string nick, std::string reason);
		void callUserConnectHooks(std::string server, std::string nick);
		void callUserQuitHooks(std::string server, std::string client, std::string nick, std::string reason);
		void callUserNickHooks(std::string server, std::string client, std::string oldNick, std::string newNick);
		void callNumericHooks(std::string server, std::string client, std::string numeric, std::vector<std::string> data);
		void callOperHooks(std::string server, std::string nick, std::string operType);
		void callSNoticeHooks(std::string server, std::string snotype, std::string message);
		void callMetadataHooks(std::string server, std::string target, std::string dataKey, std::string dataValue);
		void callServerDataHooks(std::string server, std::string dataType, std::vector<std::string> params);
		void callXLineAddHooks(std::string server, std::string lineType, std::string mask, std::string setter, time_t expiry, std::string reason);
		void callXLineRemoveHooks(std::string server, std::string lineType, std::string mask);
		void callServerConnectHooks(std::string server, std::string newServerName);
		void callServerQuitHooks(std::string server, std::string quitServerName, std::string reason);
		void callOtherDataHooks(std::string server, std::string client, std::vector<std::string> lineTokens);
		void callChanMsgOutHooks(std::string server, std::string client, std::string channel, char status, std::string message);
		void callChanMsgSendHooks(std::string server, std::string client, std::string channel, char status, std::string message);
		void callUserMsgOutHooks(std::string server, std::string client, std::string nick, std::string message);
		void callUserMsgSendHooks(std::string server, std::string client, std::string nick, std::string message);
		void callChanNoticeOutHooks(std::string server, std::string client, std::string channel, char status, std::string message);
		void callChanNoticeSendHooks(std::string server, std::string client, std::string channel, char status, std::string message);
		void callUserNoticeOutHooks(std::string server, std::string client, std::string nick, std::string message);
		void callUserNoticeSendHooks(std::string server, std::string client, std::string nick, std::string message);
		void callChanCTCPOutHooks(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params);
		void callChanCTCPSendHooks(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params);
		void callUserCTCPOutHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
		void callUserCTCPSendHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
		void callChanCTCPReplyOutHooks(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params);
		void callChanCTCPReplySendHooks(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params);
		void callUserCTCPReplyOutHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
		void callUserCTCPReplySendHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
};