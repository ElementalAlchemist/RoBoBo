#pragma once
#include "main.h"

#include "socket.h"

class Base; // This needs to be forward-declared for the Base* pointer in Module; the Base class is included below.
enum LoadResult { LOAD_SUCCESS, LOAD_ALREADYLOADED, LOAD_OPEN_ERROR, LOAD_INCOMPATIBLE, LOAD_NODEPENDS, LOAD_FAILURE };
enum Priority { PRI_HIGH, PRI_MEDIUM_HIGH, PRI_NORMAL, PRI_MEDIUM_LOW, PRI_LOW };

typedef bool MsgAction;
const bool MSG_CONTINUE = true;
const bool MSG_IGNORE = false;

class Module {
	public:
		Module(const std::string& modName, const std::map<std::string, std::string>& conf, const std::string& workDir, unsigned short debug, Base* botptr);
		virtual ~Module();
		virtual Priority priority() { return PRI_NORMAL; }
		virtual unsigned int apiVersion() = 0;
		virtual bool onLoadComplete() { return true; }
		virtual void onUnload() {}
		virtual void onRehash() {}
		virtual void onModuleLoad(const std::string& modName) {}
		virtual void onModuleUnload(const std::string& modName) {}
		virtual bool forceKeepAlive() { return false; }
		void rehash(const std::map<std::string, std::string>& conf) { config = conf; }
		void endDebug() { debugLevel = 0; }
		
		virtual MsgAction onChanMsg(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& nick, const std::string& message) { return MSG_CONTINUE; }
		virtual MsgAction onUserMsg(const std::string& server, const std::string& client, const std::string& nick, const std::string& message) { return MSG_CONTINUE; }
		virtual MsgAction onChanNotice(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& nick, const std::string& message) { return MSG_CONTINUE; }
		virtual MsgAction onUserNotice(const std::string& server, const std::string& client, const std::string& nick, const std::string& message) { return MSG_CONTINUE; }
		virtual MsgAction onChanCTCP(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& nick, const std::string& ctcp, const std::string& params) { return MSG_CONTINUE; }
		virtual MsgAction onUserCTCP(const std::string& server, const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) { return MSG_CONTINUE; }
		virtual MsgAction onChanCTCPReply(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& nick, const std::string& ctcp, const std::string& params) { return MSG_CONTINUE; }
		virtual MsgAction onUserCTCPReply(const std::string& server, const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) { return MSG_CONTINUE; }
		virtual void onChanMode(const std::string& server, const std::string& channel, bool add, const std::string& mode) {}
		virtual void onUserMode(const std::string& server, const std::string& nick, bool add, const std::string& mode) {}
		virtual void onUserSNOmask(const std::string& server, const std::string& nick, bool add, char snomask) {}
		virtual void onChanJoin(const std::string& server, const std::string& channel, const std::string& user) {}
		virtual void onChanPart(const std::string& server, const std::string& channel, const std::string& user, const std::string& reason) {}
		virtual void onChanKick(const std::string& server, const std::string& channel, const std::string& kicker, const std::string& kickee, const std::string& reason) {}
		virtual void onChanTopic(const std::string& server, const std::string& channel, const std::string& nick, const std::string& topic) {}
		virtual void onChanInvite(const std::string& server, const std::string& channel, const std::string& inviter, const std::string& invitee) {}
		virtual void onChanKnock(const std::string& server, const std::string& channel, const std::string& nick, const std::string& reason) {}
		virtual void onUserNick(const std::string& server, const std::string& oldNick, const std::string& newNick) {}
		virtual void onServerPing(const std::string& server, const std::string& sourceServer) {}
		virtual void onServerPong(const std::string& server, const std::string& sourceServer) {}
		virtual void onNumeric(const std::string& server, const std::string& numeric, const std::vector<std::string>& data) {}
		virtual void onUserOper(const std::string& server, const std::string& nick, const std::string& operType) {}
		virtual void onServerNotice(const std::string& server, char snomask, const std::string& message) {}
		virtual void onMetadata(const std::string& server, const std::string& target, const std::string& key, const std::string& value) {}
		virtual void onXLineAdd(const std::string& server, const std::string& lineType, const std::string& mask) {}
		virtual void onXLineRemove(const std::string& server, const std::string& lineType, const std::string& mask) {}
		virtual void onUserConnect(const std::string& server, const std::string& nick) {}
		virtual void onUserQuit(const std::string& server, const std::string& nick, const std::string& reason) {}
		virtual void onUserIdentChange(const std::string& server, const std::string& nick, const std::string& oldIdent, const std::string& newIdent) {}
		virtual void onUserHostChange(const std::string& server, const std::string& nick, const std::string& oldHost, const std::string& newHost) {}
		virtual void onUserGecosChange(const std::string& server, const std::string& nick, const std::string& oldGecos, const std::string& newGecos) {}
		virtual void onServerWallops(const std::string& server, const std::string& nick, const std::string& message) {}
		virtual void onServerConnect(const std::string& server, const std::string& serverName) {}
		virtual void onServerDisconnect(const std::string& server, const std::string& serverName, const std::string& reason) {}
		virtual void onServerCap(const std::string& server, const std::string& subcmd, const std::string& list) {}
		virtual void onServerCapab(const std::string& server, const std::vector<std::string>& capabList) {}
		virtual void onServerBurst(const std::string& server) {}
		virtual void onServerBurstEnd(const std::string& server) {}
		virtual void onConnect(const std::string& server) {}
		virtual void onDisconnect(const std::string& server) {}
		virtual void onOtherData(const std::string& server, const std::string& client, const std::vector<std::string>& parsedLine) {}
		
		virtual void onChanMsgOut(const std::string& server, const std::string& client, const std::string& channel, char status, std::string& message) {}
		virtual void onChanMsgSend(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& message) {}
		virtual void onUserMsgOut(const std::string& server, const std::string& client, const std::string& nick, std::string& message) {}
		virtual void onUserMsgSend(const std::string& server, const std::string& client, const std::string& nick, const std::string& message) {}
		virtual void onChanNoticeOut(const std::string& server, const std::string& client, const std::string& channel, char status, std::string& message) {}
		virtual void onChanNoticeSend(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& message) {}
		virtual void onUserNoticeOut(const std::string& server, const std::string& client, const std::string& nick, std::string& message) {}
		virtual void onUserNoticeSend(const std::string& server, const std::string& client, const std::string& nick, const std::string& message) {}
		virtual void onChanCTCPOut(const std::string& server, const std::string& client, const std::string& channel, char status, std::string& ctcp, std::string& params) {}
		virtual void onChanCTCPSend(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params) {}
		virtual void onUserCTCPOut(const std::string& server, const std::string& client, const std::string& nick, std::string& ctcp, std::string& params) {}
		virtual void onUserCTCPSend(const std::string& server, const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) {}
		virtual void onChanCTCPReplyOut(const std::string& server, const std::string& client, const std::string& channel, char status, std::string& ctcp, std::string& params) {}
		virtual void onChanCTCPReplySend(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params) {}
		virtual void onUserCTCPReplyOut(const std::string& server, const std::string& client, const std::string& nick, std::string& ctcp, std::string& params) {}
		virtual void onUserCTCPReplySend(const std::string& server, const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) {}
		
		virtual std::string description() { return "A description has not been provided by the module author."; }
		virtual std::list<std::string> provides() { return std::list<std::string> (); }
		virtual std::list<std::string> requires() { return std::list<std::string> (); }
		virtual std::list<std::string> supports() { return std::list<std::string> (); }
	protected:
		const std::string moduleName, workingDir;
		std::map<std::string, std::string> config;
		unsigned short debugLevel;
		
		void sendPrivMsg(const std::string& server, const std::string& client, const std::string& target, const std::string& message);
		void sendNotice(const std::string& server, const std::string& client, const std::string& target, const std::string& message);
		void sendCTCP(const std::string& server, const std::string& client, const std::string& target, const std::string& ctcp, const std::string& params = "");
		void sendCTCPReply(const std::string& server, const std::string& client, const std::string& target, const std::string& ctcp, const std::string& params = "");
		void setMode(const std::string& server, const std::string& client, const std::string& target, const std::list<std::string>& setModes, const std::list<std::string>& remModes);
		void setSNOmask(const std::string& server, const std::string& client, bool add, char snomask);
		void joinChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& key = "");
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
		
		void connectServer(const std::string& server);
		void disconnectServer(const std::string& server);
		LoadResult loadModule(const std::string& modName);
		void unloadModule(const std::string& modName);
		std::shared_ptr<Socket> assignSocket(const std::string& socketType);
		void rehash();
		
		unsigned int botVersion();
		std::list<std::string> activeServers();
		std::list<std::string> connectedServers();
		std::list<std::string> loadedModules();
		std::string serverType(const std::string& server);
		bool serverIsClient(const std::string& server);
		std::set<std::string> providedServices();
		bool serviceIsProvided(const std::string& service);
		std::list<std::string> serviceProviders(const std::string& service);
		std::list<std::string> serviceUsers(const std::string& service);
		void refreshServices();
		
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
		std::pair<std::string, char> compareStatus(const std::string& server, char status0, const std::string& status1);
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
	private:
		Base* bot;
};

#include "base.h"

inline void Module::sendPrivMsg(const std::string& server, const std::string& client, const std::string& target, const std::string& message) {
	bot->sendPrivMsg(server, client, target, message);
}

inline void Module::sendNotice(const std::string& server, const std::string& client, const std::string& target, const std::string& message) {
	bot->sendNotice(server, client, target, message);
}

inline void Module::sendCTCP(const std::string& server, const std::string& client, const std::string& target, const std::string& ctcp, const std::string& params) {
	bot->sendCTCP(server, client, target, ctcp, params);
}

inline void Module::sendCTCPReply(const std::string& server, const std::string& client, const std::string& target, const std::string& ctcp, const std::string& params) {
	bot->sendCTCPReply(server, client, target, ctcp, params);
}

inline void Module::setMode(const std::string& server, const std::string& client, const std::string& target, const std::list<std::string>& setModes, const std::list<std::string>& remModes) {
	bot->setMode(server, client, target, setModes, remModes);
}

inline void Module::setSNOmask(const std::string& server, const std::string& client, bool add, char snomask) {
	bot->setSNOmask(server, client, add, snomask);
}

inline void Module::joinChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& key) {
	bot->joinChan(server, client, channel, key);
}

inline void Module::partChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason) {
	bot->partChan(server, client, channel, reason);
}

inline void Module::kickUser(const std::string& server, const std::string& client, const std::string& channel, const std::string& user, const std::string& reason) {
	bot->kickUser(server, client, channel, user, reason);
}

inline std::string Module::addClient(const std::string& server, std::string& nick, std::string& ident, std::string& host, std::string& gecos) {
	return bot->addClient(server, nick, ident, host, gecos);
}

inline void Module::removeClient(const std::string& server, const std::string& client) {
	bot->removeClient(server, client);
}

inline void Module::setTopic(const std::string& server, const std::string& client, const std::string& channel, const std::string& topic) {
	bot->setTopic(server, client, channel, topic);
}

inline void Module::inviteUser(const std::string& server, const std::string& client, const std::string& channel, const std::string& user) {
	bot->inviteUser(server, client, channel, user);
}

inline void Module::knockOnChannel(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason) {
	bot->knockOnChannel(server, client, channel, reason);
}

inline void Module::changeNick(const std::string& server, const std::string& user, const std::string& newNick) {
	bot->changeNick(server, user, newNick);
}

inline void Module::sendPing(const std::string& server, const std::string& remoteServer) {
	bot->sendPing(server, remoteServer);
}

inline void Module::operUp(const std::string& server, const std::string& client, const std::string& usernameOrType, const std::string& password) {
	bot->operUp(server, client, usernameOrType, password);
}

inline void Module::sendServerNotice(const std::string& server, char snomask, const std::string& message) {
	bot->sendServerNotice(server, snomask, message);
}

inline void Module::setMetadata(const std::string& server, const std::string& target, const std::string& key, const std::string& value) {
	bot->setMetadata(server, target, key, value);
}

inline void Module::setXLine(const std::string& server, const std::string& client, const std::string& lineType, const std::string& mask, time_t duration, const std::string& reason) {
	bot->setXLine(server, client, lineType, mask, duration, reason);
}

inline void Module::remXLine(const std::string& server, const std::string& client, const std::string& lineType, const std::string& mask) {
	bot->remXLine(server, client, lineType, mask);
}

inline void Module::changeIdent(const std::string& server, const std::string& user, const std::string& newIdent) {
	bot->changeIdent(server, user, newIdent);
}

inline void Module::changeHost(const std::string& server, const std::string& user, const std::string& newHost) {
	bot->changeHost(server, user, newHost);
}

inline void Module::changeGecos(const std::string& server, const std::string& user, const std::string& newGecos) {
	bot->changeGecos(server, user, newGecos);
}

inline void Module::sendWallops(const std::string& server, const std::string& client, const std::string& message) {
	bot->sendWallops(server, client, message);
}

inline void Module::sendOtherData(const std::string& server, const std::string& client, const std::string& line) {
	bot->sendOtherData(server, client, line);
}

inline void Module::connectServer(const std::string& server) {
	bot->connectServer(server);
}

inline void Module::disconnectServer(const std::string& server) {
	bot->disconnectServer(server);
}

inline LoadResult Module::loadModule(const std::string& modName) {
	return bot->loadModule(modName);
}

inline void Module::unloadModule(const std::string& modName) {
	if (modName == moduleName)
		std::thread(&Base::unloadModule, bot, modName, true).detach();
	else
		bot->unloadModule(modName, true);
}

inline std::shared_ptr<Socket> Module::assignSocket(const std::string& socketType) {
	return bot->loadSocket(socketType);
}

inline void Module::rehash() {
	bot->rehash();
}

inline unsigned int Module::botVersion() {
	return 2900; // 3.0.0 Pre-alpha Development
}

inline std::list<std::string> Module::activeServers() {
	return bot->activeServers();
}

inline std::list<std::string> Module::connectedServers() {
	return bot->connectedServers();
}

inline std::list<std::string> Module::loadedModules() {
	return bot->loadedModules();
}

inline std::string Module::serverType(const std::string& server) {
	return bot->serverType(server);
}

inline bool Module::serverIsClient(const std::string& server) {
	return bot->serverIsClient(server);
}

inline std::set<std::string> Module::providedServices() {
	return bot->providedServices();
}

inline bool Module::serviceIsProvided(const std::string& service) {
	return bot->serviceIsProvided(service);
}

inline std::list<std::string> Module::serviceProviders(const std::string& service) {
	return bot->serviceProviders(service);
}

inline std::list<std::string> Module::serviceUsers(const std::string& service) {
	return bot->serviceUsers(service);
}

inline void Module::refreshServices() {
	bot->refreshServices(moduleName, this);
}

inline std::list<std::string> Module::networkServerList(const std::string& server) {
	return bot->networkServerList(server);
}

inline std::list<std::string> Module::xLineTypes(const std::string& server) {
	return bot->xLineTypes(server);
}

inline std::list<std::string> Module::xLineList(const std::string& server, const std::string& lineType) {
	return bot->xLineList(server, lineType);
}

inline time_t Module::xLineExpiry(const std::string& server, const std::string& lineType, const std::string& mask) {
	return bot->xLineExpiry(server, lineType, mask);
}

inline std::string Module::xLineReason(const std::string& server, const std::string& lineType, const std::string& mask) {
	return bot->xLineReason(server, lineType, mask);
}

inline std::list<std::string> Module::chanListModes(const std::string& server) {
	return bot->chanListModes(server);
}

inline std::list<std::string> Module::chanParamModes(const std::string& server) {
	return bot->chanParamModes(server);
}

inline std::list<std::string> Module::chanNoParamModes(const std::string& server) {
	return bot->chanNoParamModes(server);
}

inline std::list<std::pair<std::string, char>> Module::chanPrefixes(const std::string& server) {
	return bot->chanPrefixes(server);
}

inline std::pair<std::string, char> Module::compareStatus(const std::string& server, const std::string& status0, const std::string& status1) {
	return bot->compareStatus(server, status0, status1);
}

inline std::pair<std::string, char> Module::compareStatus(const std::string& server, const std::string& status0, char status1) {
	return bot->compareStatus(server, status0, status1);
}

inline std::pair<std::string, char> Module::compareStatus(const std::string& server, char status0, const std::string& status1) {
	return bot->compareStatus(server, status1, status0);
}

inline std::pair<std::string, char> Module::compareStatus(const std::string& server, char status0, char status1) {
	return bot->compareStatus(server, status0, status1);
}

inline std::string Module::chanTopic(const std::string& server, const std::string& channel) {
	return bot->chanTopic(server, channel);
}

inline time_t Module::chanTimestamp(const std::string& server, const std::string& channel) {
	return bot->chanTimestamp(server, channel);
}

inline std::set<std::string> Module::chanUsers(const std::string& server, const std::string& channel) {
	return bot->chanUsers(server, channel);
}

inline bool Module::userInChan(const std::string& server, const std::string& channel, const std::string& user) {
	return bot->userInChan(server, channel, user);
}

inline std::pair<std::string, char> Module::userStatus(const std::string& server, const std::string& channel, const std::string& user) {
	return bot->userStatus(server, channel, user);
}

inline bool Module::userHasStatus(const std::string& server, const std::string& channel, const std::string& user, const std::string& status) {
	return bot->userHasStatus(server, channel, user, status);
}

inline bool Module::userHasStatus(const std::string& server, const std::string& channel, const std::string& user, char status) {
	return bot->userHasStatus(server, channel, user, status);
}

inline bool Module::userHasStatusOrGreater(const std::string& server, const std::string& channel, const std::string& user, const std::string& status) {
	return bot->userHasStatusOrGreater(server, channel, user, status);
}

inline bool Module::userHasStatusOrGreater(const std::string& server, const std::string& channel, const std::string& user, char status) {
	return bot->userHasStatusOrGreater(server, channel, user, status);
}

inline std::list<std::string> Module::chanModes(const std::string& server, const std::string& channel) {
	return bot->chanModes(server, channel);
}

inline std::list<std::string> Module::chanListModeList(const std::string& server, const std::string& channel, const std::string& listMode) {
	return bot->chanListModeList(server, channel, listMode);
}

inline bool Module::chanHasMode(const std::string& server, const std::string& channel, const std::string& mode) {
	return bot->chanHasMode(server, channel, mode);
}

inline std::string Module::chanModeParam(const std::string& server, const std::string& channel, const std::string& mode) {
	return bot->chanModeParam(server, channel, mode);
}

inline std::list<std::string> Module::clientList(const std::string& server) {
	return bot->clientList(server);
}

inline std::string Module::clientNick(const std::string& server, const std::string& client) {
	return bot->clientNick(server, client);
}

inline std::string Module::userIdent(const std::string& server, const std::string& user) {
	return bot->userIdent(server, user);
}

inline std::string Module::userHost(const std::string& server, const std::string& user) {
	return bot->userHost(server, user);
}

inline std::string Module::userGecos(const std::string& server, const std::string& user) {
	return bot->userGecos(server, user);
}

inline std::set<std::string> Module::userModes(const std::string& server, const std::string& user) {
	return bot->userModes(server, user);
}

inline bool Module::userHasMode(const std::string& server, const std::string& user, const std::string& mode) {
	return bot->userHasMode(server, user, mode);
}

inline std::set<char> Module::userSNOmasks(const std::string& server, const std::string& user) {
	return bot->userSNOmasks(server, user);
}

inline bool Module::userHasSNOmask(const std::string& server, const std::string& user, char snomask) {
	return bot->userHasSNOmask(server, user, snomask);
}

inline std::set<std::string> Module::userChans(const std::string& server, const std::string& user) {
	return bot->userChans(server, user);
}

inline time_t Module::userTimestamp(const std::string& server, const std::string& user) {
	return bot->userTimestamp(server, user);
}

inline time_t Module::userNickTimestamp(const std::string& server, const std::string& user) {
	return bot->userNickTimestamp(server, user);
}