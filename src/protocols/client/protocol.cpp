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
		std::shared_ptr<Client> newClient (new Client (id, std::move(nick), std::move(ident), std::move(gecos), std::move(password), std::move(socket), this));
		clients.insert(std::pair<std::string, std::shared_ptr<Client>> (id, newClient));
		users.insert(std::pair<std::string, std::shared_ptr<User>> (id, static_cast<std::shared_ptr<User>>(newClient)));
	}
}

bool Protocol::connected() {
	bool connectedClientsExist = false;
	std::list<std::string> removeClients;
	for (auto client : clients) {
		if (client.second->checkConnection())
			connectedClientsExist = true;
		else if (client.second->wantsToReconnect())
			client.second->doReconnect();
		else
			removeClients.push_back(client.first);
	}
	for (auto clientID : removeClients) {
		clients.erase(clientID);
		auto clientUser = users.find(clientID);
		if (clientUser != users.end())
			users.erase(clientUser);
	}
	return connectedClientsExist;
}

bool Protocol::shouldUnload() {
	return clients.empty();
}

void Protocol::disconnect(const std::string& reason) {
	LogManager* logger = LogManager::getHandle();
	logger->log(LOG_DEFAULT, "protocol-client-connection-" + serverName, "Disconnecting all clients");
	for (auto client : clients)
		client.second->disconnect(reason);
}

void Protocol::onRehash() {
	Config* config = Config::getHandle();
	std::unordered_map<std::string, std::string> configBlock = config->getSingleBlockOnConditions("server", std::unordered_map<std::string, std::string> { { "name", serverName } });
	bool newFloodThrottle = Config::makeBool(configBlock["floodthrottle"]);
	if (newFloodThrottle != floodThrottle) {
		floodThrottle = newFloodThrottle;
		if (floodThrottle) {
			for (auto client : clients)
				client.second->startFloodThrottle();
		} else {
			for (auto client : clients)
				client.second->endFloodThrottle();
		}
	}
}

void Protocol::sendMsg(const std::string& client, const std::string& target, const std::string& message, const std::map<std::string, std::string>& tags) {
	auto clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	IRCMessage msg ("PRIVMSG");
	msg.setParams(std::vector<std::string> (2));
	msg.setParam(0, convertCommaSeparatedTargetList(target));
	auto tagIter = tags.find("intents");
	if (tagIter == tags.end() || capabilities.find("intents") != capabilities.end()) {
		msg.setParam(1, message);
		msg.setTags(tags);
	} else {
		std::map<std::string, std::string> newTags (tags);
		newTags.erase("intents");
		msg.setTags(newTags);
		msg.setParam(1, "\x01" + tagIter->second + " " + message + "\x01");
	}
	clientIter->second->sendLine(&msg);
}

void Protocol::sendNotice(const std::string& client, const std::string& target, const std::string& message, const std::map<std::string, std::string>& tags) {
	auto clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	IRCMessage msg ("NOTICE");
	msg.setParams(std::vector<std::string> (2));
	msg.setParam(0, convertCommaSeparatedTargetList(target));
	auto tagIter = tags.find("intents");
	if (tagIter == tags.end() || capabilities.find("intents") != capabilities.end()) {
		msg.setParam(1, message);
		msg.setTags(tags);
	} else {
		std::map<std::string, std::string> newTags (tags);
		newTags.erase("intents");
		msg.setTags(newTags);
		msg.setParam(1, "\x01" + tagIter->second + " " + message + "\x01");
	}
	clientIter->second->sendLine(&msg);
}

void Protocol::setMode(const std::string& client, const std::string& target, const std::list<std::tuple<bool, std::string, std::string>>& modes, const std::map<std::string, std::string>& tags) {
	auto clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	IRCMessage msg ("MODE");
	std::vector<std::string> modeParams;
	bool adding = std::get<0>(modes.front());
	std::string modeStr (adding ? "+" : "-");
	for (auto mode : modes) {
		if (std::get<0>(mode) != adding) {
			adding = std::get<0>(mode);
			modeStr += (adding ? "+" : "-");
		}
		modeStr += std::get<1>(mode);
		std::string param (std::get<2>(mode));
		if (!param.empty())
			modeParams.push_back(param);
	}
	std::vector<std::string> cmdParams { target };
	cmdParams.reserve(2 + modeParams.size());
	cmdParams.push_back(modeStr);
	for (auto param : modeParams)
		cmdParams.push_back(param);
	msg.setParams(cmdParams);
	msg.setTags(tags);
	clientIter->second->sendLine(&msg);
}

void Protocol::joinChan(const std::string& client, const std::string& channel, const std::map<std::string, std::string>& tags) {
	auto clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	IRCMessage msg ("JOIN");
	msg.setParams(std::vector<std::string> { channel });
	msg.setTags(tags);
	clientIter->second->sendLine(&msg);
}

void Protocol::joinChan(const std::string& client, const std::string& channel, const std::string& key, const std::map<std::string, std::string>& tags) {
	auto clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	IRCMessage msg ("JOIN");
	msg.setParams(std::vector<std::string> { channel, key });
	msg.setTags(tags);
	clientIter->second->sendLine(&msg);
}

void Protocol::partChan(const std::string& client, const std::string& channel, const std::string& reason, const std::map<std::string, std::string>& tags) {
	auto clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	IRCMessage msg ("PART");
	msg.setParams(std::vector<std::string> { channel, reason });
	msg.setTags(tags);
	clientIter->second->sendLine(&msg);
}

void Protocol::kickUser(const std::string& client, const std::string& channel, const std::string& user, const std::string& reason, const std::map<std::string, std::string>& tags) {
	auto clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	IRCMessage msg ("KICK");
	msg.setParams(std::vector<std::string> { channel, user, reason });
	msg.setTags(tags);
	clientIter->second->sendLine(&msg);
}

void Protocol::setTopic(const std::string& client, const std::string& channel, const std::string& topic, const std::map<std::string, std::string>& tags) {
	auto clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	IRCMessage msg ("TOPIC");
	msg.setParams(std::vector<std::string> { channel, topic });
	msg.setTags(tags);
	clientIter->second->sendLine(&msg);
}

void Protocol::inviteUser(const std::string& client, const std::string& channel, const std::string& user, const std::map<std::string, std::string>& tags) {
	auto clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	IRCMessage msg ("INVITE");
	msg.setParams(std::vector<std::string> { user, channel });
	msg.setTags(tags);
	clientIter->second->sendLine(&msg);
}

void Protocol::knock(const std::string& client, const std::string& channel, const std::string& reason, const std::map<std::string, std::string>& tags) {
	auto clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	IRCMessage msg ("KNOCK");
	msg.setParams(std::vector<std::string> { channel, reason });
	msg.setTags(tags);
	clientIter->second->sendLine(&msg);
}

void Protocol::changeNick(const std::string& client, const std::string& newNick, const std::map<std::string, std::string>& tags) {
	auto clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	IRCMessage msg ("NICK");
	msg.setParams(std::vector<std::string> { newNick });
	msg.setTags(tags);
	clientIter->second->sendLine(&msg);
}

void Protocol::sendPing(const std::string& client, const std::string& data, const std::map<std::string, std::string>& tags) {
	auto clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	IRCMessage msg ("PING");
	msg.setParams(std::vector<std::string> { data });
	msg.setTags(tags);
	clientIter->second->sendLine(&msg);
}

void Protocol::setAway(const std::string& client, const std::string& reason, const std::map<std::string, std::string>& tags) {
	auto clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	IRCMessage msg ("AWAY");
	msg.setParams(std::vector<std::string> { reason });
	msg.setTags(tags);
	clientIter->second->sendLine(&msg);
}

void Protocol::setUnaway(const std::string& client, const std::map<std::string, std::string>& tags) {
	auto clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	IRCMessage msg ("AWAY");
	msg.setTags(tags);
	clientIter->second->sendLine(&msg);
}

void Protocol::oper(const std::string& client, const std::string& username, const std::string& password, const std::map<std::string, std::string>& tags) {
	auto clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	IRCMessage msg ("OPER");
	msg.setParams(std::vector<std::string> { username, password });
	msg.setTags(tags);
	clientIter->second->sendLine(&msg);
}

void Protocol::sendWallops(const std::string& client, const std::string& message, const std::map<std::string, std::string>& tags) {
	auto clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	IRCMessage msg ("WALLOPS");
	msg.setParams(std::vector<std::string> { message });
	msg.setTags(tags);
	clientIter->second->sendLine(&msg);
}

void Protocol::sendOtherData(const std::string& client, const IRCMessage* line) {
	auto clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	clientIter->second->sendLine(line);
}

std::string Protocol::addClient(const std::string& nick, const std::string& ident, const std::string& gecos, const std::string& password, const std::string& socket) {
	if (nick.empty() || ident.empty() || gecos.empty())
		return "";
	if (socket.empty()) {
		Config* config = Config::getHandle();
		std::unordered_map<std::string, std::string> configBlock = config->getSingleBlockOnConditions("server", std::unordered_map<std::string, std::string> { { "name", serverName } });
		socket = configBlock["defaultsocket"];
		if (socket.empty())
			return "";
	}
	std::string id (getNextID());
	std::shared_ptr<Client> newClient (new Client (id, std::move(nick), std::move(ident), std::move(gecos), std::move(password), std::move(socket), this));
	clients.insert(std::pair<std::string, std::shared_ptr<Client>> (id, newClient));
	users.insert(std::pair<std::string, std::shared_ptr<User>> (id, static_cast<std::shared_ptr<User>>(newClient)));
	return id;
}

void Protocol::removeClient(const std::string& client, const std::string& reason) {
	auto clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	clientIter->second->disconnect(reason);
	users.erase(clientIter->first);
	clients.erase(clientIter);
}

std::set<std::string> Protocol::serverCapabilities() {
	return capabilities;
}

std::set<char> Protocol::chanTypes() {
	return channelTypes;
}

std::list<std::pair<ModeType, std::string>> Protocol::allChanModes() {
	std::list<std::pair<ModeType, std::string>> chanModes;
	for (auto mode : serverChanModes) {
		if (mode.first != MODE_STATUS)
			chanModes.push_back(mode);
	}
	return chanModes;
}

ModeType Protocol::chanModeType(const std::string& mode) {
	auto modeTypeIter = chanModeType.find(mode);
	if (modeTypeIter == chanModeType.end())
		return MODE_NOPARAM;
	return modeTypeIter->second;
}

char Protocol::prefixSymbol(const std::string& mode) {
	auto prefixIter = chanPrefixModeToSymbol.find(mode);
	if (prefixIter == chanPrefixModeToSymbol.end())
		return ' ';
	return prefixIter->second;
}

std::pair<std::string, char> Protocol::compareStatus(const std::string& status0, const std::string& status1) {
	for (auto prefix : chanPrefixOrder) {
		if (prefix == status0) {
			char symbol = chanPrefixModeToSymbol[status0];
			return std::pair<std::string, char> (status0, symbol);
		}
		if (prefix == status1) {
			char symbol = chanPrefixModeToSymbol[status1];
			return std::pair<std::string, char> (status1, symbol);
		}
	}
	return std::pair<std::string, char> ("", ' ');
}

std::pair<std::string, char> Protocol::compareStatus(const std::string& status0, char status1) {
	auto prefixIter = chanPrefixSymbolToMode.find(status1);
	if (prefixIter == chanPrefixSymbolToMode.end()) {
		auto otherPrefixIter = chanPrefixModeToSymbol.find(status0);
		if (otherPrefixIter == chanPrefixModeToSymbol.end())
			return std::pair<std::string, char> ("", ' ');
		return std::pair<std::string, char> (status0, otherPrefixIter->second);
	}
	return compareStatus(status0, prefixIter->second);
}

std::pair<std::string, char> Protocol::compareStatus(char status0, char status1) {
	auto prefixIter0 = chanPrefixSymbolToMode.find(status0);
	auto prefixIter1 = chanPrefixSymbolToMode.find(status1);
	if (prefixIter0 == chanPrefixSymbolToMode.end() && prefixIter1 == chanPrefixSymbolToMode.end())
		return std::pair<std::string, char> ("", ' ');
	if (prefixIter0 == chanPrefixSymbolToMode.end())
		return std::pair<std::string, char> (prefixIter1->second, status1);
	if (prefixIter1 == chanPrefixSymbolToMode.end())
		return std::pair<std::string, char> (prefixIter0->second, status0);
	return compareStatus(prefixIter0->second, prefixIter1->second);
}

std::string Protocol::chanTopic(const std::string& channel) {
	auto chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return "";
	return chanIter->second->topic();
}

std::string Protocol::chanTopicSetter(const std::string& channel) {
	auto chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return "";
	return chanIter->second->topicSetter();
}

time_t Protocol::chanTopicTimestamp(const std::string& channel) {
	auto chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return 0;
	return chanIter->second->topicTime();
}

time_t Protocol::chanTimestamp(const std::string& channel) {
	auto chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return 0;
	return chanIter->second->time();
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

std::string Protocol::convertCommaSeparatedTargetList(std::string targets) {
	std::list<std::string> targetList;
	while (!targets.empty()) {
		size_t commaPos = targets.find(',');
		targetList.push_back(targets.substr(0, commaPos));
		if (commaPos == std::string::npos)
			targets.clear();
		else
			targets = targets.substr(commaPos + 1);
	}
	for (std::string oneTarget : targetList) {
		auto userIter = users.find(oneTarget);
		if (userIter != users.end())
			targets += "," + userIter->second.nick();
		else
			targets += "," + oneTarget;
	}
	return targets.substr(1);
}