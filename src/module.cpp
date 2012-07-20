#include "module.h"

Module::Module(const std::string& modName, const std::map<std::string, std::string>& conf, const std::string& workDir, unsigned short debug, Base* botptr) : moduleName(modName), workingDir(workDir), config(conf), debugLevel(debug), bot(botptr) {}

Module::~Module() {}
// This is just here because we need a virtual destructor.

inline void Module::sendPrivMsg(const std::string& server, const std::string& client, const std::string& target, const std::string& message) {
	bot->sendPrivMsg(server, client, target, message);
}

inline void Module::sendNotice(const std::string& server, const std::string& client, const std::string& target, const std::string& message) {
	bot->sendNotice(server, client, target, message);
}

inline void Module::sendCTCP(const std::string& server, const std::string& client, const std::string& target, const std::string& ctcp, const std::string& params = "") {
	bot->sendCTCP(server, client, target, ctcp, params);
}

inline void Module::sendCTCPReply(const std::string& server, const std::string& client, const std::string& target, const std::string& ctcp, const std::string& params = "") {
	bot->sendCTCPReply(server, client, target, ctcp, params);
}

inline void Module::setMode(const std::string& server, const std::string& client, const std::string& target, const std::list<std::string>& setModes, const std::list<std::string>& remModes) {
	bot->setMode(server, client, target, setModes, remModes);
}

inline void Module::setSNOmask(const std::string& server, const std::string& client, bool add, char snomask) {
	bot->setSNOmask(server, client, add, snomask);
}

inline void Module::joinChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& key = "") {
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

inline bool Module::userHasSNOmask(const std::string& server, const std::string& user, const std::string& mode) {
	return bot->userHasSNOmask(server, user, mode);
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
