#include "protocol.h"

Protocol::Protocol() : floodThrottle(true), nextID(0) {}

void Protocol::connectServer() {
	LogManager* logger = LogManager::getHandle();
	logger->log(LOG_DEBUG, "protocol-client-connect-" + serverName, "Connecting to " + serverName + " ...");
	Config* config = Config::getHandle();
	std::unordered_map<std::string, std::string> configBlock = config->getSingleBlockOnConditions("server", std::unordered_map<std::string, std::string> { { "name", serverName } });
	serverAddress = configBlock["address"];
	serverPort = configBlock["port"];
	serverBindAddr = configBlock["bind"];
	floodThrottle = Config::makeBool(configBlock["floodthrottle"]);
	if (serverAddress.empty() || serverPort.empty()) {
		logger->log(LOG_ERROR, "protocol-client-connect-" + serverName, "Cannot connect to " + serverName + " because the configuration is incomplete or invalid.");
		throw ServerBadConfiguration ();
	}
	std::list<std::unordered_map<std::string, std::string>> clientConfigs = config->getBlocksOnConditions("client", std::unordered_map<std::string, std::string> { { "server", serverName } });
	for (auto clientConf : clientConfigs) {
		std::string nick (clientConf["nick"]);
		std::string ident (clientConf["ident"]);
		std::string gecos (clientConf["gecos"]);
		std::string password (clientConf["password"]);
		std::string socket (clientConf["socket"]);
		if (nick.empty() || ident.empty() || gecos.empty() || socket.empty()) {
			logger->log(LOG_ERROR, "protocol-client-connect-" + serverName, "An improperly configured client was encountered.  Client not added.");
			continue;
		}
		std::string id (clientConf["id"]);
		if (id.empty()) {
			while (users.find(id) != users.end())
				id = getNextID();
		}
		std::shared_ptr<Client> newClient (new Client (std::move(id), std::move(nick), std::move(ident), std::move(gecos), std::move(password), std::move(socket), this));
		clients.insert(std::pair<std::string, std::shared_ptr<Client>> (id, newClient));
		users.insert(std::pair<std::string, std::shared_ptr<User>> (id, static_cast<std::shared_ptr<User>>(newClient)));
	}
}

bool Protocol::connected() {
	
}

bool Protocol::shouldUnload() {
	
}

void Protocol::disconnect() {
	
}

void Protocol::onRehash() {
	
}

void Protocol::sendMsg(const std::string& client, const std::string& target, const std::string& message, const std::map<std::string, std::string>& tags) {
	
}

void Protocol::sendNotice(const std::string& client, const std::string& target, const std::string& message, const std::map<std::string, std::string>& tags) {
	
}

void Protocol::setMode(const std::string& client, const std::string& target, const std::list<std::tuple<bool, std::string, std::string>>& modes, const std::map<std::string, std::string>& tags) {
	
}

void Protocol::joinChan(const std::string& client, const std::string& channel, const std::map<std::string, std::string>& tags) {
	
}

void Protocol::joinChan(const std::string& client, const std::string& channel, const std::string& key, const std::map<std::string, std::string>& tags) {
	
}

void Protocol::partChan(const std::string& client, const std::string& channel, const std::string& reason, const std::map<std::string, std::string>& tags) {
	
}

void Protocol::kickUser(const std::string& client, const std::string& channel, const std::string& user, const std::string& reason, const std::map<std::string, std::string>& tags) {
	
}

void Protocol::setTopic(const std::string& client, const std::string& channel, const std::string& topic, const std::map<std::string, std::string>& tags) {
	
}

void Protocol::inviteUser(const std::string& client, const std::string& channel, const std::string& user, const std::map<std::string, std::string>& tags) {
	
}

void Protocol::knock(const std::string& client, const std::string& channel, const std::string& reason, const std::map<std::string, std::string>& tags) {
	
}

void Protocol::changeNick(const std::string& client, const std::string& newNick, const std::map<std::string, std::string>& tags) {
	
}

void Protocol::sendPing(const std::string& client, const std::string& data, const std::map<std::string, std::string>& tags) {
	
}

void Protocol::setAway(const std::string& client, const std::string& reason, const std::map<std::string, std::string>& tags) {
	
}

void Protocol::setUnaway(const std::string& client, const std::map<std::string, std::string>& tags) {
	
}

void Protocol::oper(const std::string& client, const std::string& username, const std::string& password, const std::map<std::string, std::string>& tags) {
	
}

void Protocol::sendWallops(const std::string& client, const std::string& message, const std::map<std::string, std::string>& tags) {
	
}

void Protocol::sendOtherData(const std::string& client, const IRCMessage* line) {
	
}

std::string Protocol::addClient(const std::string& nick, const std::string& ident, const std::string& gecos) {
	
}

void Protocol::removeClient(const std::string& client) {
	
}

std::set<std::string> Protocol::serverCapabilities() {
	
}

std::set<char> Protocol::chanTypes() {
	
}

std::list<std::pair<ModeType, std::string>> Protocol::allChanModes() {
	
}

ModeType Protocol::chanModeType(const std::string& mode) {
	
}

char Protocol::prefixSymbol(const std::string& mode) {
	
}

std::pair<std::string, char> Protocol::compareStatus(const std::string& status0, const std::string& status1) {
	
}

std::pair<std::string, char> Protocol::compareStatus(const std::string& status0, char status1) {
	
}

std::pair<std::string, char> Protocol::compareStatus(char status0, char status1) {
	
}

std::string Protocol::chanTopic(const std::string& channel) {
	
}

std::string Protocol::chanTopicSetter(const std::string& channel) {
	
}

time_t Protocol::chanTopicTimestamp(const std::string& channel) {
	
}

time_t Protocol::chanTimestamp(const std::string& channel) {
	
}

std::list<std::string> Protocol::chanUsers(const std::string& channel) {
	
}

bool Protocol::userInChan(const std::string& channel, const std::string& user) {
	
}

std::pair<std::string, char> Protocol::userStatus(const std::string& channel, const std::string& user) {
	
}

bool Protocol::userHasStatus(const std::string& channel, const std::string& user, const std::string& status) {
	
}

bool Protocol::userHasStatus(const std::string& channel, const std::string& user, char status) {
	
}

bool Protocol::userHasStatusOrGreater(const std::string& channel, const std::string& user, const std::string& status) {
	
}

bool Protocol::userHasStatusOrGreater(const std::string& channel, const std::string& user, char status) {
	
}

std::map<std::string, std::string> Protocol::chanModes(const std::string& channel) {
	
}

bool Protocol::chanHasMode(const std::string& channel, const std::string& mode) {
	
}

std::string Protocol::chanModeParam(const std::string& channel, const std::string& mode) {
	
}

std::list<std::string> Protocol::chanListModeList(const std::string& channel, const std::string& mode) {
	
}

std::list<std::string> Protocol::clientList() {
	
}

std::string Protocol::userNick(const std::string& user) {
	
}

std::string Protocol::userIdent(const std::string& user) {
	
}

std::string Protocol::userHost(const std::string& user) {
	
}

std::string Protocol::userGecos(const std::string& user) {
	
}

std::string Protocol::idFromNick(const std::string& nick) {
	
}

std::list<std::pair<ModeType, std::string>> Protocol::allUserModes() {
	
}

ModeType Protocol::userModeType(const std::string& mode) {
	
}

std::map<std::string, std::string> Protocol::userModes(const std::string& user) {
	
}

bool Protocol::userHasMode(const std::string& user, const std::string& mode) {
	
}

std::string Protocol::userModeParam(const std::string& user, const std::string& mode) {
	
}

std::list<std::string> Protocol::userListModeList(const std::string& user, const std::string& listMode) {
	
}

std::set<std::string> Protocol::userChans(const std::string& user) {
	
}

std::string Protocol::servName() {
	return serverName;
}

std::shared_ptr<Socket> Protocol::obtainSocket(const std::string& sockType) {
	return assignSocket(sockType);
}

void Protocol::connectSocket(const std::shared_ptr<Socket> sock) {
	sock->connectServer(serverAddress, serverPort, serverBindAddr);
}

bool Protocol::floodThrottleInEffect() {
	return floodThrottle;
}

void Protocol::processIncoming(const std::string& client, const IRCMessage* message) {
	MutexLocker mutexLock (&processMutex);
	// TODO: the rest of this
}

std::string Protocol::getNextID() {
	std::ostringstream currID;
	currID << nextID;
	nextID++;
	return currID.str();
}