#pragma once
#include "main.h"
#include <dlfcn.h>

#include "socket.h"
#include "protocol.h"
#include "module.h"

// The spawn function needs a typedef so that it can be called after a dlsym.
// This typedef is used to cast the result of dlsym so that it can be used. (see Base::loadModule)
typedef void* (module_spawn_t)(std::string, std::map<std::string, std::string>, std::string, unsigned short, Base*);
typedef void* (protocol_spawn_t)(std::string, std::map<std::string, std::string>, std::string, bool, unsigned short, Base*);
typedef void* (socket_spawn_t)();

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
		void disconnectServer(std::string server, std::string reason = "");
		std::shared_ptr<Socket> loadSocket(std::string sockettype);
		void unloadSocket(std::string sockettype, Socket* socketptr);
		void rehash();
		void sigRehash(); // This version is called when we receive the signal to rehash rather than when requested by a module
		void endDebug();
		
		void sendPrivMsg(const std::string& server, const std::string& client, const std::string& target, const std::string& message);
		void sendNotice(const std::string& server, const std::string& client, const std::string& target, const std::string& message);
		void sendCTCP(const std::string& server, const std::string& client, const std::string& target, const std::string& ctcp, const std::string& params);
		void sendCTCPReply(const std::string& server, const std::string& client, const std::string& target, const std::string& ctcp, const std::string& params);
		void setMode(const std::string& server, const std::string& client, const std::string& target, const std::list<std::string>& setModes, const std::list<std::string>& remModes);
		void setSNOmask(const std::string& server, const std::string& client, bool add, char snomask);
		void joinChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& key);
		void partChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason);
		void kickUser(const std::string& server, const std::string& client, const std::string& channel, const std::string& user, const std::string& reason);
		std::string addClient(const std::string& server, std::string& nick, std::string& ident, std::string& host, std::string& gecos);
		void removeClient(const std::string& server, const std::string& client);
		void setTopic(const std::string& server, const std::string& client, const std::string& channel, const std::string& topic);
		void inviteUser(const std::string& server, const std::string& client, const std::string& channel, const std::string& user);
		void knockOnChannel(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason);
		void changeNick(const std::string& server, const std::string& user, const std::string& newNick);
		void sendPing(const std::string& server, const std::string& remoteServer);
		void operUp(const std::string& server, const std::string& client, const std::string& usernameOrType, const std::string& password);
		void sendServerNotice(const std::string& server, char snomask, const std::string& message);
		void setMetadata(const std::string& server, const std::string& target, const std::string& key, const std::string& value);
		void setXLine(const std::string& server, const std::string& client, const std::string& lineType, const std::string& mask, time_t duration, const std::string& reason);
		void remXLine(const std::string& server, const std::string& client, const std::string& lineType, const std::string& mask);
		void changeIdent(const std::string& server, const std::string& user, const std::string& newIdent);
		void changeHost(const std::string& server, const std::string& user, const std::string& newHost);
		void changeGecos(const std::string& server, const std::string& user, const std::string& newGecos);
		void sendWallops(const std::string& server, const std::string& client, const std::string& message);
		void sendOtherData(const std::string& server, const std::string& client, const std::string& line);
		
		std::list<std::string> activeServers();
		std::list<std::string> connectedServers();
		std::list<std::string> loadedModules();
		std::string serverType(const std::string& server);
		bool serverIsClient(const std::string& server);
		std::set<std::string> providedServices();
		bool serviceIsProvided(const std::string& service);
		std::list<std::string> serviceProviders(const std::string& service);
		std::list<std::string> serviceUsers(const std::string& service);
		void refreshServices(const std::string& modName, Module* modptr);
		
		std::list<std::string> networkServerList(const std::string& server);
		std::list<std::string> xLineTypes(const std::string& server);
		std::list<std::string> xLineList(const std::string& server, const std::string& lineType);
		time_t xLineExpiry(const std::string& server, const std::string& lineType, const std::string& mask);
		std::string xLineReason(const std::string& server, const std::string& lineType, const std::string& mask);
		std::list<std::string> chanListModes(const std::string& server);
		std::list<std::string> chanParamModes(const std::string& server);
		std::list<std::string> chanNoParamModes(const std::string& server);
		std::list<std::pair<std::string, char>> chanPrefixes(const std::string& server);
		std::pair<std::string, char> compareStatus(const std::string& server, const std::string& status0, const std::string& status1);
		std::pair<std::string, char> compareStatus(const std::string& server, const std::string& status0, char status1);
		std::pair<std::string, char> compareStatus(const std::string& server, char status0, char status1);
		
		std::string chanTopic(const std::string& server, const std::string& channel);
		time_t chanTimestamp(const std::string& server, const std::string& channel);
		std::set<std::string> chanUsers(const std::string& server, const std::string& channel);
		bool userInChan(const std::string& server, const std::string& channel, const std::string& user);
		std::pair<std::string, char> userStatus(const std::string& server, const std::string& channel, const std::string& user);
		bool userHasStatus(const std::string& server, const std::string& channel, const std::string& user, const std::string& status);
		bool userHasStatus(const std::string& server, const std::string& channel, const std::string& user, char status);
		bool userHasStatusOrGreater(const std::string& server, const std::string& channel, const std::string& user, const std::string& status);
		bool userHasStatusOrGreater(const std::string& server, const std::string& channel, const std::string& user, char status);
		std::list<std::string> chanModes(const std::string& server, const std::string& channel);
		std::list<std::string> chanListModeList(const std::string& server, const std::string& channel, const std::string& listMode);
		bool chanHasMode(const std::string& server, const std::string& channel, const std::string& mode);
		std::string chanModeParam(const std::string& server, const std::string& channel, const std::string& mode);
		
		std::list<std::string> clientList(const std::string& server);
		std::string clientNick(const std::string& server, const std::string& client);
		std::string userIdent(const std::string& server, const std::string& user);
		std::string userHost(const std::string& server, const std::string& user);
		std::string userGecos(const std::string& server, const std::string& user);
		std::set<std::string> userModes(const std::string& server, const std::string& user);
		bool userHasMode(const std::string& server, const std::string& user, const std::string& mode);
		std::set<char> userSNOmasks(const std::string& server, const std::string& user);
		bool userHasSNOmask(const std::string& server, const std::string& user, char snomask);
		std::set<std::string> userChans(const std::string& server, const std::string& user);
		time_t userTimestamp(const std::string& server, const std::string& user);
		time_t userNickTimestamp(const std::string& server, const std::string& user);
		
		void modChanMsgHook(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& nick, const std::string& message) { std::thread(&Base::callChanMsgHooks, this, server, client, channel, status, nick, message).detach(); }
		void modUserMsgHook(const std::string& server, const std::string& client, const std::string& nick, const std::string& message) { std::thread(&Base::callUserMsgHooks, this, server, client, nick, message).detach(); }
		void modChanNoticeHook(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& nick, const std::string& message) { std::thread(&Base::callChanNoticeHooks, this, server, client, channel, status, nick, message).detach(); }
		void modUserNoticeHook(const std::string& server, const std::string& client, const std::string& nick, const std::string& message) { std::thread(&Base::callUserNoticeHooks, this, server, client, nick, message).detach(); }
		void modChanCTCPHook(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& nick, const std::string& ctcp, const std::string& params) { std::thread(&Base::callChanCTCPHooks, this, server, client, channel, status, nick, ctcp, params).detach(); }
		void modUserCTCPHook(const std::string& server, const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) { std::thread(&Base::callUserCTCPHooks, this, server, client, nick, ctcp, params).detach(); }
		void modChanCTCPReplyHook(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& nick, const std::string& ctcp, const std::string& params) { std::thread(&Base::callChanCTCPReplyHooks, this, server, client, channel, status, nick, ctcp, params).detach(); }
		void modUserCTCPReplyHook(const std::string& server, const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) { std::thread(&Base::callUserCTCPReplyHooks, this, server, client, nick, ctcp, params).detach(); }
		void modChanModeHook(const std::string& server, const std::string& channel, bool add, const std::string& mode) { std::thread(&Base::callChanModeHooks, this, server, channel, add, mode).detach(); }
		void modUserModeHook(const std::string& server, const std::string& nick, bool add, const std::string& mode) { std::thread(&Base::callUserModeHooks, this, server, nick, add, mode).detach(); }
		void modUserSNOmaskHook(const std::string& server, const std::string& nick, bool add, char snomask) { std::thread(&Base::callUserSNOmaskHooks, this, server, nick, add, snomask).detach(); }
		void modChanJoinHook(const std::string& server, const std::string& channel, const std::string& user) { std::thread(&Base::callChanJoinHooks, this, server, channel, user).detach(); }
		void modChanPartHook(const std::string& server, const std::string& channel, const std::string& user, const std::string& reason) { std::thread(&Base::callChanPartHooks, this, server, channel, user, reason).detach(); }
		void modChanKickHook(const std::string& server, const std::string& channel, const std::string& kicker, const std::string& kickee, const std::string& reason) { std::thread(&Base::callChanKickHooks, this, server, channel, kicker, kickee, reason).detach(); }
		void modChanTopicHook(const std::string& server, const std::string& channel, const std::string& nick, const std::string& topic) { std::thread(&Base::callChanTopicHooks, this, server, channel, nick, topic).detach(); }
		void modChanInviteHook(const std::string& server, const std::string& channel, const std::string& inviter, const std::string& invitee) { std::thread(&Base::callChanInviteHooks, this, server, channel, inviter, invitee).detach(); }
		void modChanKnockHook(const std::string& server, const std::string& channel, const std::string& nick, const std::string& reason) { std::thread(&Base::callChanKnockHooks, this, server, channel, nick, reason).detach(); }
		void modUserNickHook(const std::string& server, const std::string& oldNick, const std::string& newNick) { std::thread(&Base::callUserNickHooks, this, server, oldNick, newNick).detach(); }
		void modServerPingHook(const std::string& server, const std::string& sourceServer) { std::thread(&Base::callServerPingHooks, this, server, sourceServer).detach(); }
		void modServerPongHook(const std::string& server, const std::string& sourceServer) { std::thread(&Base::callServerPongHooks, this, server, sourceServer).detach(); }
		void modNumericHook(const std::string& server, const std::string& numeric, std::vector<std::string> data) { std::thread(&Base::callNumericHooks, this, server, numeric, data).detach(); }
		void modUserOperHook(const std::string& server, const std::string& nick, const std::string& operType) { std::thread(&Base::callUserOperHooks, this, server, nick, operType).detach(); }
		void modServerNoticeHook(const std::string& server, char snomask, const std::string& message) { std::thread(&Base::callServerNoticeHooks, this, server, snomask, message).detach(); }
		void modMetadataHook(const std::string& server, const std::string& target, const std::string& key, const std::string& value) { std::thread(&Base::callMetadataHooks, this, server, target, key, value).detach(); }
		void modXLineAddHook(const std::string& server, const std::string& lineType, const std::string& mask) { std::thread(&Base::callXLineAddHooks, this, server, lineType, mask).detach(); }
		void modXLineRemoveHook(const std::string& server, const std::string& lineType, const std::string& mask) { std::thread(&Base::callXLineRemoveHooks, this, server, lineType, mask).detach(); }
		void modUserConnectHook(const std::string& server, const std::string& nick) { std::thread(&Base::callUserConnectHooks, this, server, nick).detach(); }
		void modUserQuitHook(const std::string& server, const std::string& nick, const std::string& reason) { std::thread(&Base::callUserQuitHooks, this, server, nick, reason).detach(); }
		void modUserIdentChangeHook(const std::string& server, const std::string& nick, const std::string& oldIdent, const std::string& newIdent) { std::thread(&Base::callUserIdentChangeHooks, this, server, nick, oldIdent, newIdent).detach(); }
		void modUserHostChangeHook(const std::string& server, const std::string& nick, const std::string& oldHost, const std::string& newHost) { std::thread(&Base::callUserHostChangeHooks, this, server, nick, oldHost, newHost).detach(); }
		void modUserGecosChangeHook(const std::string& server, const std::string& nick, const std::string& oldGecos, const std::string& newGecos) { std::thread(&Base::callUserGecosChangeHooks, this, server, nick, oldGecos, newGecos).detach(); }
		void modServerWallopsHook(const std::string& server, const std::string& nick, const std::string& message) { std::thread(&Base::callServerWallopsHooks, this, server, nick, message).detach(); }
		void modServerConnectHook(const std::string& server, const std::string& serverName) { std::thread(&Base::callServerConnectHooks, this, server, serverName).detach(); }
		void modServerDisconnectHook(const std::string& server, const std::string& serverName, const std::string& reason) { std::thread(&Base::callServerDisconnectHooks, this, server, serverName, reason).detach(); }
		void modServerCapHook(const std::string& server, const std::string& subcmd, const std::string& list) { std::thread(&Base::callServerCapHooks, this, server, subcmd, list).detach(); }
		void modServerCapabHook(const std::string& server, std::vector<std::string> capabList) { std::thread(&Base::callServerCapabHooks, this, server, capabList).detach(); }
		void modServerBurstHook(const std::string& server) { std::thread(&Base::callServerBurstHooks, this, server).detach(); }
		void modServerBurstEndHook(const std::string& server) { std::thread(&Base::callServerBurstEndHooks, this, server).detach(); }
		void modOtherDataHook(const std::string& server, const std::string& client, const std::vector<std::string>& parsedLine) { std::thread(&Base::callOtherDataHooks, this, server, client, parsedLine).detach(); }
		
		void modChanMsgOutHook(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& message, Protocol* protoptr) { std::thread(&Base::callChanMsgOutHooks, this, server, client, channel, status, message, protoptr).detach(); }
		void modChanMsgSendHook(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& message) { std::thread(&Base::callChanMsgSendHooks, this, server, client, channel, status, message).detach(); }
		void modUserMsgOutHook(const std::string& server, const std::string& client, const std::string& nick, const std::string& message, Protocol* protoptr) { std::thread(&Base::callUserMsgOutHooks, this, server, client, nick, message, protoptr).detach(); }
		void modUserMsgSendHook(const std::string& server, const std::string& client, const std::string& nick, const std::string& message) { std::thread(&Base::callUserMsgSendHooks, this, server, client, nick, message).detach(); }
		void modChanNoticeOutHook(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& message, Protocol* protoptr) { std::thread(&Base::callChanNoticeOutHooks, this, server, client, channel, status, message, protoptr).detach(); }
		void modChanNoticeSendHook(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& message) { std::thread(&Base::callChanNoticeSendHooks, this, server, client, channel, status, message).detach(); }
		void modUserNoticeOutHook(const std::string& server, const std::string& client, const std::string& nick, const std::string& message, Protocol* protoptr) { std::thread(&Base::callUserNoticeOutHooks, this, server, client, nick, message, protoptr).detach(); }
		void modUserNoticeSendHook(const std::string& server, const std::string& client, const std::string& nick, const std::string& message) { std::thread(&Base::callUserNoticeSendHooks, this, server, client, nick, message).detach(); }
		void modChanCTCPOutHook(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params, Protocol* protoptr) { std::thread(&Base::callChanCTCPOutHooks, this, server, client, channel, status, ctcp, params, protoptr).detach(); }
		void modChanCTCPSendHook(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params) { std::thread(&Base::callChanCTCPSendHooks, this, server, client, channel, status, ctcp, params).detach(); }
		void modUserCTCPOutHook(const std::string& server, const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params, Protocol* protoptr) { std::thread(&Base::callUserCTCPOutHooks, this, server, client, nick, ctcp, params, protoptr).detach(); }
		void modUserCTCPSendHook(const std::string& server, const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) { std::thread(&Base::callUserCTCPSendHooks, this, server, client, nick, ctcp, params).detach(); }
		void modChanCTCPReplyOutHook(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params, Protocol* protoptr) { std::thread(&Base::callChanCTCPReplyOutHooks, this, server, client, channel, status, ctcp, params, protoptr).detach(); }
		void modChanCTCPReplySendHook(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params) { std::thread(&Base::callChanCTCPReplySendHooks, this, server, client, channel, status, ctcp, params).detach(); }
		void modUserCTCPReplyOutHook(const std::string& server, const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params, Protocol* protoptr) { std::thread(&Base::callUserCTCPReplyOutHooks, this, server, client, nick, ctcp, params, protoptr).detach(); }
		void modUserCTCPReplySendHook(const std::string& server, const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) { std::thread(&Base::callUserCTCPReplySendHooks, this, server, client, nick, ctcp, params).detach(); }
	private:
		const std::string workingDir, configDir, configName;
		const unsigned short debugLevel;
		const bool logDump;
		bool startup;
		std::map<std::string, std::map<std::string, std::string>> serverConfig, moduleConfig;
		std::list<std::string> startupServers, startupModules;
		std::map<std::string, Protocol*> servers;
		std::map<std::string, Module*> highModules, mediumHighModules, normalModules, mediumLowModules, lowModules;
		std::unordered_map<std::string, void*> moduleFiles;
		std::map<std::string, Priority> modulePriority;
		std::map<std::string, std::list<std::string>> moduleServices, moduleRequires, moduleSupports;
		std::map<std::string, std::string> moduleDescriptions;
		std::unordered_map<std::string, void*> protocolFiles;
		std::unordered_map<std::string, std::set<std::string>> protocolTypes;
		std::unordered_map<std::string, void*> socketFiles;
		std::unordered_map<std::string, size_t> socketCounts;
		std::mutex modHookMutex;
		
		// These hooks are called as separate threads, and we don't want the references potentially changing on us
		// (or worse, going out of scope and destructing), so these get parameters passed by value.
		void callChanMsgHooks(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message);
		void callUserMsgHooks(std::string server, std::string client, std::string nick, std::string message);
		void callChanNoticeHooks(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message);
		void callUserNoticeHooks(std::string server, std::string client, std::string nick, std::string message);
		void callChanCTCPHooks(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string params);
		void callUserCTCPHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
		void callChanCTCPReplyHooks(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string params);
		void callUserCTCPReplyHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
		void callChanModeHooks(std::string server, std::string channel, bool add, std::string mode);
		void callUserModeHooks(std::string server, std::string nick, bool add, std::string mode);
		void callUserSNOmaskHooks(std::string server, std::string nick, bool add, char snomask);
		void callChanJoinHooks(std::string server, std::string channel, std::string user);
		void callChanPartHooks(std::string server, std::string channel, std::string user, std::string reason);
		void callChanKickHooks(std::string server, std::string channel, std::string kicker, std::string kickee, std::string reason);
		void callChanTopicHooks(std::string server, std::string channel, std::string nick, std::string topic);
		void callChanInviteHooks(std::string server, std::string channel, std::string inviter, std::string invitee);
		void callChanKnockHooks(std::string server, std::string channel, std::string nick, std::string reason);
		void callUserNickHooks(std::string server, std::string oldNick, std::string newNick);
		void callServerPingHooks(std::string server, std::string sourceServer);
		void callServerPongHooks(std::string server, std::string sourceServer);
		void callNumericHooks(std::string server, std::string numeric, std::vector<std::string> data);
		void callUserOperHooks(std::string server, std::string nick, std::string operType);
		void callServerNoticeHooks(std::string server, char snomask, std::string message);
		void callMetadataHooks(std::string server, std::string target, std::string key, std::string value);
		void callXLineAddHooks(std::string server, std::string lineType, std::string mask);
		void callXLineRemoveHooks(std::string server, std::string lineType, std::string mask);
		void callUserConnectHooks(std::string server, std::string nick);
		void callUserQuitHooks(std::string server, std::string nick, std::string reason);
		void callUserIdentChangeHooks(std::string server, std::string nick, std::string oldIdent, std::string newIdent);
		void callUserHostChangeHooks(std::string server, std::string nick, std::string oldHost, std::string newHost);
		void callUserGecosChangeHooks(std::string server, std::string nick, std::string oldGecos, std::string newGecos);
		void callServerWallopsHooks(std::string server, std::string nick, std::string message);
		void callServerConnectHooks(std::string server, std::string serverName);
		void callServerDisconnectHooks(std::string server, std::string serverName, std::string reason);
		void callServerCapHooks(std::string server, std::string subcmd, std::string list);
		void callServerCapabHooks(std::string server, std::vector<std::string> capabList);
		void callServerBurstHooks(std::string server);
		void callServerBurstEndHooks(std::string server);
		void callOtherDataHooks(std::string server, std::string client, std::vector<std::string> parsedLine);
		
		void callChanMsgOutHooks(std::string server, std::string client, std::string channel, char status, std::string message, Protocol* protoptr);
		void callChanMsgSendHooks(std::string server, std::string client, std::string channel, char status, std::string message);
		void callUserMsgOutHooks(std::string server, std::string client, std::string nick, std::string message, Protocol* protoptr);
		void callUserMsgSendHooks(std::string server, std::string client, std::string nick, std::string message);
		void callChanNoticeOutHooks(std::string server, std::string client, std::string channel, char status, std::string message, Protocol* protoptr);
		void callChanNoticeSendHooks(std::string server, std::string client, std::string channel, char status, std::string message);
		void callUserNoticeOutHooks(std::string server, std::string client, std::string nick, std::string message, Protocol* protoptr);
		void callUserNoticeSendHooks(std::string server, std::string client, std::string nick, std::string message);
		void callChanCTCPOutHooks(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params, Protocol* protoptr);
		void callChanCTCPSendHooks(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params);
		void callUserCTCPOutHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string params, Protocol* protoptr);
		void callUserCTCPSendHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
		void callChanCTCPReplyOutHooks(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params, Protocol* protoptr);
		void callChanCTCPReplySendHooks(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params);
		void callUserCTCPReplyOutHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string params, Protocol* protoptr);
		void callUserCTCPReplySendHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
		
		// A manager class to automatically release mutexes makes mutex handling easier and more reliable
		class MutexManager {
			public:
				MutexManager(std::mutex* mutexPtr);
				~MutexManager();
				MutexManager(const MutexManager&) = delete;
				MutexManager& operator=(MutexManager const&) = delete;
				void release();
			private:
				std::mutex* mutex;
				bool active;
		};
};