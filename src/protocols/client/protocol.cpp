#include "protocol.h"

Protocol::Protocol() : floodThrottle(true), loaded(true), nextID(0), maxModes(1), maxTargets(1) {}

Protocol::~Protocol() {
	loaded = false;
	if (dataThread.joinable())
		dataThread.join();
}

void Protocol::connectServer() {
	LogManager* logger = LogManager::getHandle();
	logger->log(LOG_DEBUG, "protocol-client-connect-" + serverName, "Connecting to " + serverName + " ...");
	Config* config = Config::getHandle();
	std::unordered_map<std::string, std::string> configBlock = config->getSingleBlockOnConditions("server", std::unordered_map<std::string, std::string> { { "name", serverName } });
	serverAddress = configBlock["address"];
	serverPort = configBlock["port"];
	serverBindAddr = configBlock["bind"];
	floodThrottle = Config::makeBool(configBlock["floodthrottle"]);
	loadModeNamesAndDefaults(configBlock);
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
		newClient->connect();
		callHook(HOOK_CLIENT_CONNECT_SELF, id);
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
	MutexLocker mutexLock (&processMutex);
	for (auto client : clients)
		client.second->disconnect(reason);
	loaded = false;
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
	if (target.empty())
		return; // Clearly we can't send this message anywhere.
	std::list<std::string> targets = convertCommaSeparatedList(target);
	std::list<std::string> targetLists;
	while (!targets.empty()) {
		std::list<std::string> targetSubset;
		while (!targets.empty() && targetSubset.size() < maxTargets) {
			targetSubset.push_back(targets.front());
			targets.erase(targets.begin());
		}
		targetLists.push_back(convertListToCommaSeparatedString(targetSubset));
	}
	IRCMessage msg ("PRIVMSG");
	msg.setParams(std::vector<std::string> (2));
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
	for (std::string& msgTarget : targetLists) {
		msg.setParam(0, msgTarget);
		clientIter->second->sendLine(&msg);
	}
}

void Protocol::sendNotice(const std::string& client, const std::string& target, const std::string& message, const std::map<std::string, std::string>& tags) {
	auto clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	if (target.empty())
		return;
	std::list<std::string> targets = convertCommaSeparatedList(target);
	std::list<std::string> targetLists;
	while (!targets.empty()) {
		std::list<std::string> targetSubset;
		while (!targets.empty() && targetSubset.size() < maxTargets) {
			targetSubset.push_back(targets.front());
			targets.erase(targets.begin());
		}
		targetLists.push_back(convertListToCommaSeparatedString(targetSubset));
	}
	IRCMessage msg ("NOTICE");
	msg.setParams(std::vector<std::string> (2));
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
	for (std::string& msgTarget : targetLists) {
		msg.setParam(0, msgTarget);
		clientIter->second->sendLine(&msg);
	}
}

void Protocol::setMode(const std::string& client, const std::string& target, std::list<std::tuple<bool, std::string, std::string>> modes, const std::map<std::string, std::string>& tags) {
	auto clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	IRCMessage msg ("MODE");
	msg.setTags(tags);
	while (!modes.empty()) {
		std::list<std::tuple<bool, std::string, std::string>> modesThisTime;
		while (!modes.empty() && modesThisTime.size() < maxModes) {
			modesThisTime.push_back(modes.front());
			modes.erase(modes.begin());
		}
		std::vector<std::string> modeParams;
		bool adding = std::get<0>(modesThisTime.front());
		std::string modeStr (adding ? "+" : "-");
		for (auto mode : modesThisTime) {
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
		clientIter->second->sendLine(&msg);
	}
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
	while (users.find(id) != users.end())
		id = getNextID();
	std::shared_ptr<Client> newClient (new Client (id, std::move(nick), std::move(ident), std::move(gecos), std::move(password), std::move(socket), this));
	clients.insert(std::pair<std::string, std::shared_ptr<Client>> (id, newClient));
	users.insert(std::pair<std::string, std::shared_ptr<User>> (id, static_cast<std::shared_ptr<User>>(newClient)));
	newClient->connect();
	callHook(HOOK_CLIENT_CONNECT_SELF, id);
	return id;
}

void Protocol::removeClient(const std::string& client, const std::string& reason) {
	auto clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	MutexLocker mutexLock (&processMutex);
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
	auto modeTypeIter = serverChanModeType.find(mode);
	if (modeTypeIter == serverChanModeType.end())
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
	auto chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return std::list<std::string> ();
	return chanIter->second->users();
}

bool Protocol::userInChan(const std::string& channel, const std::string& user) {
	auto chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return false;
	return chanIter->second->userInChan(user);
}

std::pair<std::string, char> Protocol::userStatus(const std::string& channel, const std::string& user) {
	auto chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return std::pair<std::string, char> ("", ' ');
	std::list<std::string> statuses = chanIter->second->statuses(user);
	if (statuses.empty())
		return std::pair<std::string, char> ("", ' ');
	char symbol = chanPrefixModeToSymbol.find(statuses.front())->second;
	return std::pair<std::string, char> (statuses.front(), symbol);
}

bool Protocol::userHasStatus(const std::string& channel, const std::string& user, const std::string& status) {
	auto chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return false;
	return chanIter->second->userHasStatus(user, status);
}

bool Protocol::userHasStatus(const std::string& channel, const std::string& user, char status) {
	auto chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return false;
	auto prefixIter = chanPrefixSymbolToMode.find(status);
	if (prefixIter == chanPrefixSymbolToMode.end())
		return false;
	return chanIter->second->userHasStatus(user, prefixIter->second);
}

bool Protocol::userHasStatusOrGreater(const std::string& channel, const std::string& user, const std::string& status) {
	auto chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return false;
	return chanIter->second->userHasStatusOrGreater(user, status);
}

bool Protocol::userHasStatusOrGreater(const std::string& channel, const std::string& user, char status) {
	auto chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return false;
	auto prefixIter = chanPrefixSymbolToMode.find(status);
	if (prefixIter == chanPrefixSymbolToMode.end())
		return false;
	return chanIter->second->userHasStatusOrGreater(user, prefixIter->second);
}

std::map<std::string, std::string> Protocol::chanModes(const std::string& channel) {
	auto chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return std::map<std::string, std::string> ();
	return chanIter->second->modes();
}

bool Protocol::chanHasMode(const std::string& channel, const std::string& mode) {
	auto chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return false;
	return chanIter->second->modeSet(mode);
}

std::string Protocol::chanModeParam(const std::string& channel, const std::string& mode) {
	auto chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return "";
	return chanIter->second->modeParam(mode);
}

std::list<std::string> Protocol::chanListModeList(const std::string& channel, const std::string& mode) {
	auto chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return std::list<std::string> ();
	return chanIter->second->modeList(mode);
}

std::list<std::string> Protocol::clientList() {
	std::list<std::string> listOfClients;
	for (auto client : clients)
		listOfClients.push_back(client.first);
	return listOfClients;
}

std::string Protocol::userNick(const std::string& user) {
	auto userIter = users.find(user);
	if (userIter == users.end())
		return "";
	return userIter->second->nick();
}

std::string Protocol::userIdent(const std::string& user) {
	auto userIter = users.find(user);
	if (userIter == users.end())
		return "";
	return userIter->second->ident();
}

std::string Protocol::userHost(const std::string& user) {
	auto userIter = users.find(user);
	if (userIter == users.end())
		return "";
	return userIter->second->host();
}

std::string Protocol::userGecos(const std::string& user) {
	auto userIter = users.find(user);
	if (userIter == users.end())
		return "";
	return userIter->second->gecos();
}

std::string Protocol::idFromNick(const std::string& nick) {
	auto nickIter = nickToID.find(nick);
	if (nickIter == nickToID.end())
		return nick;
	return nickIter->second;
}

std::list<std::pair<ModeType, std::string>> Protocol::allUserModes() {
	std::list<std::pair<ModeType, std::string>> modes;
	for (auto mode : serverUserModes)
		modes.push_back(mode);
	return modes;
}

ModeType Protocol::userModeType(const std::string& mode) {
	auto typeIter = serverUserModeType.find(mode);
	if (typeIter == serverUserModeType.end())
		return MODE_NOPARAM;
	return typeIter->second;
}

std::map<std::string, std::string> Protocol::userModes(const std::string& user) {
	auto clientIter = clients.find(user);
	if (clientIter == clients.end())
		return std::map<std::string, std::string> ();
	return clientIter->second->modes();
}

bool Protocol::userHasMode(const std::string& user, const std::string& mode) {
	auto clientIter = clients.find(user);
	if (clientIter == clients.end())
		return false;
	return clientIter->second->modeSet(mode);
}

std::string Protocol::userModeParam(const std::string& user, const std::string& mode) {
	auto clientIter = clients.find(user);
	if (clientIter == clients.end())
		return "";
	return clientIter->second->modeParam(mode);
}

std::list<std::string> Protocol::userListModeList(const std::string& user, const std::string& listMode) {
	auto clientIter = clients.find(user);
	if (clientIter == clients.end())
		return std::list<std::string> ();
	return clientIter->second->modeList(listMode);
}

std::set<std::string> Protocol::userChans(const std::string& user) {
	auto userIter = users.find(user);
	if (userIter == users.end())
		return std::set<std::string> ();
	return userIter->second->channels();
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
	receivedData.push(std::pair<std::string, std::unique_ptr<IRCMessage>> (client, std::unique_ptr<IRCMessage> (new IRCMessage (*message))));
}

std::string Protocol::getNextID() {
	std::ostringstream currID;
	currID << nextID;
	nextID++;
	return currID.str();
}

std::list<std::string> Protocol::convertCommaSeparatedList(std::string str) {
	std::list<std::string> list;
	while (!str.empty()) {
		size_t commaPos = str.find(',');
		list.push_back(str.substr(0, commaPos));
		if (commaPos == std::string::npos)
			str.clear();
		else
			str = str.substr(commaPos + 1);
	}
	return list;
}

std::string Protocol::convertListToCommaSeparatedString(const std::list<std::string>& list) {
	std::string str;
	for (std::string oneTarget : list) {
		auto userIter = users.find(oneTarget);
		if (userIter != users.end())
			str += "," + userIter->second.nick();
		else
			str += "," + oneTarget;
	}
	return str.substr(1);
}

void Protocol::loadModeNamesAndDefaults(std::unordered_map<std::string, std::string> modeConfig) {
	// The parameter for this function is intentionally copied instead of referenced.
	
	channelTypes.insert('#');
	
	saveChanMode("admin", 'a', modeConfig["cmode/admin"]);
	saveChanMode("allowinvite", 'A', modeConfig["cmode/allowinvite"]);
	saveChanMode("auditorium", 'u', modeConfig["cmode/auditorium"]);
	saveChanMode("autoop", 'w', modeConfig["cmode/autoop"]);
	saveChanMode("ban", 'b', ""); // The names of RFC-specified modes may not be edited
	saveChanMode("banexception", 'e', modeConfig["cmode/banexception"]);
	saveChanMode("blockcaps", 'B', modeConfig["cmode/blockcaps"]);
	saveChanMode("blockcolor", 'c', modeConfig["cmode/blockcolor"]);
	saveChanMode("c_registered", 'r', modeConfig["cmode/c_registered"]);
	saveChanMode("censor", 'G', modeConfig["cmode/censor"]);
	saveChanMode("delayjoin", 'D', modeConfig["cmode/delayjoin"]);
	saveChanMode("delaymsg", 'd', modeConfig["cmode/delaymsg"]);
	saveChanMode("exemptchanops", 'X', modeConfig["cmode/exemptchanops"]);
	saveChanMode("filter", 'g', modeConfig["cmode/filter"]);
	saveChanMode("flood", 'f', modeConfig["cmode/flood"]);
	saveChanMode("founder", 'q', modeConfig["cmode/founder"]);
	saveChanMode("halfop", 'h', modeConfig["cmode/halfop"]);
	saveChanMode("history", 'H', modeConfig["cmode/history"]);
	saveChanMode("invex", 'I', modeConfig["cmode/invex"]);
	saveChanMode("inviteonly", 'i', ""); // The names of RFC-specified modes may not be edited
	saveChanMode("joinflood", 'j', modeConfig["cmode/joinflood"]);
	saveChanMode("key", 'k', ""); // The names of RFC-specified modes may not be edited
	saveChanMode("kicknorejoin", 'J', modeConfig["cmode/kicknorejoin"]);
	saveChanMode("limit", 'l', ""); // The names of RFC-specified modes may not be edited
	saveChanMode("moderated", 'm', ""); // The names of RFC-specified modes may not be edited
	saveChanMode("namebase", 'Z', modeConfig["cmode/namebase"]); // NOTE: Even though this is here, there is no special handling for it in this module
	saveChanMode("nickflood", 'F', modeConfig["cmode/nickflood"]);
	saveChanMode("noctcp", 'C', modeConfig["cmode/noctcp"]);
	saveChanMode("noextmsg", 'n', ""); // The names of RFC-specified modes may not be edited
	saveChanMode("nokick", 'Q', modeConfig["cmode/nokick"]);
	saveChanMode("noknock", 'K', modeConfig["cmode/noknock"]);
	saveChanMode("nonick", 'N', modeConfig["cmode/nonick"]);
	saveChanMode("nonotice", 'T', modeConfig["cmode/nonotice"]);
	saveChanMode("official-join", 'Y', modeConfig["cmode/official-join"]);
	saveChanMode("op", 'o', ""); // The names of RFC-specified modes may not be edited
	saveChanMode("operonly", 'O', modeConfig["cmode/operonly"]);
	saveChanMode("operprefix", 'y', modeConfig["cmode/operprefix"]);
	saveChanMode("permanent", 'P', modeConfig["cmode/permanent"]);
	saveChanMode("private", 'p', ""); // The names of RFC-specified modes may not be edited
	saveChanMode("quiet", ' ', modeConfig["cmode/quiet"]); // Here to provide the mode name, but is not by default entered
	saveChanMode("redirect", 'L', modeConfig["cmode/redirect"]);
	saveChanMode("reginvite", 'R', modeConfig["cmode/reginvite"]);
	saveChanMode("regmoderated", 'M', modeConfig["cmode/regmoderated"]);
	saveChanMode("repeat", 'E', modeConfig["cmode/repeat"]);
	saveChanMode("secret", 's', ""); // The names of RFC-specified modes may not be edited
	saveChanMode("sslonly", 'z', modeConfig["cmode/sslonly"]);
	saveChanMode("stripcolor", 'S', modeConfig["cmode/stripcolor"]);
	saveChanMode("topiclock", 't', ""); // The names of RFC-specified modes may not be edited
	saveChanMode("voice", 'v', ""); // The names of RFC-specified modes may not be edited
	
	saveUserMode("antiredirect", 'L', modeConfig["umode/antiredirect"]);
	saveUserMode("bot", 'B', modeConfig["umode/bot"]);
	saveUserMode("callerid", 'g', modeConfig["umode/callerid"]);
	saveUserMode("cloak", 'x', modeConfig["umode/cloak"]);
	saveUserMode("deaf", 'd', modeConfig["umode/deaf"]);
	saveUserMode("deaf_commonchan", 'c', modeConfig["umode/deaf_commonchan"]);
	saveUserMode("helpop", 'h', modeConfig["umode/helpop"]);
	saveUserMode("hidechans", 'I', modeConfig["umode/hidechans"]);
	saveUserMode("hideoper", 'H', modeConfig["umode/hideoper"]);
	saveUserMode("invisible", 'i', ""); // The names of RFC-specified modes may not be edited
	saveUserMode("oper", 'o', ""); // The names of RFC-specified modes may not be edited
	saveUserMode("regdeaf", 'R', modeConfig["umode/regdeaf"]);
	saveUserMode("servprotect", 'k', modeConfig["umode/servprotect"]);
	saveUserMode("showwhois", 'W', modeConfig["umode/showwhois"]);
	saveUserMode("snomask", 's', modeConfig["umode/snomask"]);
	saveUserMode("u_censor", 'G', modeConfig["umode/u_censor"]);
	saveUserMode("u_registered", 'r', modeConfig["umode/u_registered"]);
	saveUserMode("u_stripcolor", 'S', modeConfig["umode/u_stripcolor"]);
	saveUserMode("wallops", 'w', ""); // The names of RFC-specified modes may not be edited
	
	// Fill in default channel modes
	// This list will get wiped and rewritten when we read 005 CHANMODES if the server presents it
	serverChanModes.insert(std::pair<ModeType, std::string> (MODE_LIST, "ban"));
	serverChanModeType.insert(std::pair<std::string, ModeType> ("ban", MODE_LIST));
	serverChanModes.insert(std::pair<ModeType, std::string> (MODE_NOPARAM, "inviteonly"));
	serverChanModeType.insert(std::pair<std::string, ModeType> ("inviteonly", MODE_NOPARAM));
	serverChanModes.insert(std::pair<ModeType, std::string> (MODE_PARAM_UNSET, "key"));
	serverChanModeType.insert(std::pair<std::string, ModeType> ("key", MODE_PARAM_UNSET));
	serverChanModes.insert(std::pair<ModeType, std::string> (MODE_PARAM, "limit"));
	serverChanModeType.insert(std::pair<std::string, ModeType> ("limit", MODE_PARAM));
	serverChanModes.insert(std::pair<ModeType, std::string> (MODE_NOPARAM, "moderated"));
	serverChanModeType.insert(std::pair<std::string, ModeType> ("moderated", MODE_NOPARAM));
	serverChanModes.insert(std::pair<ModeType, std::string> (MODE_NOPARAM, "noextmsg"));
	serverChanModeType.insert(std::pair<std::string, ModeType> ("noextmsg", MODE_NOPARAM));
	serverChanModes.insert(std::pair<ModeType, std::string> (MODE_STATUS, "op"));
	serverChanModeType.insert(std::pair<std::string, ModeType> ("op", MODE_STATUS));
	serverChanModes.insert(std::pair<ModeType, std::string> (MODE_NOPARAM, "private"));
	serverChanModeType.insert(std::pair<std::string, ModeType> ("private", MODE_NOPARAM));
	serverChanModes.insert(std::pair<ModeType, std::string> (MODE_NOPARAM, "secret"));
	serverChanModeType.insert(std::pair<std::string, ModeType> ("secret", MODE_NOPARAM));
	serverChanModes.insert(std::pair<ModeType, std::string> (MODE_NOPARAM, "topiclock"));
	serverChanModeType.insert(std::pair<std::string, ModeType> ("topiclock", MODE_NOPARAM));
	serverChanModes.insert(std::pair<ModeType, std::string> (MODE_STATUS, "voice"));
	serverChanModeType.insert(std::pair<std::string, ModeType> ("voice", MODE_STATUS));
	// Don't forget to also put the statuses in their own special status place
	chanPrefixModeToSymbol.insert(std::pair<std::string, char> ("op", '@'));
	chanPrefixSymbolToMode.insert(std::pair<char, std::string> ('@', "op"));
	chanPrefixModeToSymbol.insert(std::pair<std::string, char> ("voice", '+'));
	chanPrefixSymbolToMode.insert(std::pair<char, std::string> ('+', "voice"));
	chanPrefixOrder = std::list<std::string> { "op", "voice" };
	
	// Fill in default user modes
	// This list will get wiped and rewritten when we read the user modes from 004
	// (assuming all but +s is MODE_NOPARAM) or if we read 005 USERMODES
	serverUserModes.insert(std::pair<ModeType, std::string> (MODE_NOPARAM, "invisible"));
	serverUserModeType.insert(std::pair<std::string, ModeType> ("invisible", MODE_NOPARAM));
	serverUserModes.insert(std::pair<ModeType, std::string> (MODE_NOPARAM, "oper"));
	serverUserModeType.insert(std::pair<std::string, ModeType> ("oper", MODE_NOPARAM));
	serverUserModes.insert(std::pair<ModeType, std::string> (MODE_NOPARAM, "wallops"));
	serverUserModeType.insert(std::pair<std::string, ModeType> ("wallops", MODE_NOPARAM));
	if (chanModeStrToChar.find("snomask") != chanModeStrToChar.end()) {
		// Assume snomask is a MODE_PARAM type (as it is on most servers) unless we get a 005 USERMODES entry
		serverUserModes.insert(std::pair<ModeType, std::string> (MODE_PARAM, "snomask"));
		serverUserModeType.insert(std::pair<std:string, ModeType> ("snomask", MODE_PARAM));
	}
}

void Protocol::saveChanMode(const std::string& name, char letter, const std::string& override) {
	std::string lowerOverride;
	std::transform(override.begin(), override.end(), std::back_inserter(lowerOverride), ::tolower);
	if (lowerOverride == "none" || lowerOverride == "disable")
		return;
	LogManager* logger = LogManager::getHandle();
	if (!override.empty())
		letter = override[0]; // Save the config-specified letter instead
	if (letter == ' ')
		return;
	if (chanModeCharToStr.find(letter) != chanModeCharToStr.end()) {
		std::ostringstream logMsg;
		logMsg << "Cannot assign channel mode '" << letter << "' to mode " << name << ": Letter already assigned to another mode.";
		logger->log(LOG_DEFAULT, "protocol-client-setup-" + serverName, logMsg.str());
		return;
	}
	chanModeStrToChar[name] = letter;
	chanModeCharToStr[letter] = name;
}

void Protocol::saveUserMode(const std::string& name, char letter, const std::string& override) {
	std::string lowerOverride;
	std::transform(override.begin(), override.end(), std::back_inserter(lowerOverride), ::tolower);
	if (lowerOverride == "none" || lowerOverride == "disable")
		return;
	LogManager* logger = LogManager::getHandle();
	if (!override.empty())
		letter = override[0]; // Save the config-specified letter insteda
	if (userModeCharToStr.find(letter) != userModeCharToStr.end()) {
		std::ostringstream logMsg;
		logMsg << "Cannot assign user mode '" << letter "' to mode " << name << ": Letter already assigned to another mode.";
		logger->log(LOG_DEFAULT, "protocol-client-setup-" + serverName, logMsg.str());
		return;
	}
	userModeStrToChar[name] = letter;
	userModeCharToStr[letter] = name;
}

ModeType Protocol::selectModeTypeFrom005ModePosition(unsigned int pos) {
	switch (pos) {
		case 0:
			return MODE_LIST;
		case 1:
			return MODE_PARAM_UNSET;
		case 2:
			return MODE_PARAM;
		default:
			return MODE_NOPARAM;
	}
}

void Protocol::parse005Prefix(const std::string& prefixDescriptor) {
	std::string modes (prefixDescriptor.substr(1)); // Don't use the opening parenthesis
	size_t parenPos = modes.find(')'); // The closing parenthesis is now splitting modes and symbols
	std::string symbols (modes.substr(parenPos + 1));
	modes = modes.substr(0, parenPos);
	if (modes.size() != symbols.size())
		return; // Something has gone horribly wrong with the server.
	chanPrefixModeToSymbol.clear();
	chanPrefixSymbolToMode.clear();
	chanPrefixOrder.clear();
	for (size_t i = 0; i < modes.size(); i++) {
		auto modeIter = chanModeCharToStr.find(modes[i]);
		std::string mode;
		if (modeIter == chanModeCharToStr.end())
			mode = std::string(modes[i]);
		else
			mode = modeIter->second;
		chanPrefixOrder.push_back(mode);
		chanPrefixModeToSymbol.insert(std::pair<std::string, char> (mode, symbols[i]));
		chanPrefixSymbolToMode.insert(std::pair<char, std::string> (symbols[i], mode));
	}
}

void Protocol::parse005ChanModes(const std::string& channelModeDescriptor) {
	serverChanModes.clear();
	serverChanModeType.clear();
	for (std::string& status : chanPrefixOrder) {
		serverChanModes.insert(std::pair<ModeType, std::string> (MODE_STATUS, status));
		serverChanModeType.insert(std::pair<std::string, ModeType> (status, MODE_STATUS));
	}
	unsigned int catNum = 0;
	ModeType currType = selectModeTypeFrom005ModePosition(catNum);
	for (char mode : channelModeDescriptor) {
		if (mode == ',') {
			catNum++;
			currType = selectModeTypeFrom005ModePosition(catNum);
			continue;
		}
		auto modeIter = chanModeCharToStr.find(mode);
		std::string modeName;
		if (modeIter == chanModeCharToStr.end())
			modeName = std::string(mode);
		else
			modeName = modeIter->second;
		serverChanModes.insert(std::pair<ModeType, std::string> (currType, modeName));
		serverChanModeType.insert(std::pair<std::string, ModeType> (modeName, currType));
	}
}

void Protocol::parse005UserModes(const std::string& userModeDescriptor) {
	serverUserModes.clear();
	serverUserModeType.clear();
	unsigned int catNum = 0;
	ModeType currType = selectModeTypeFrom005ModePosition(catNum);
	for (char mode : userModeDescriptor) {
		if (mode == ',') {
			catNum++;
			currType = selectModeTypeFrom005ModePosition(catNum);
			continue;
		}
		auto modeIter = userModeCharToStr.find(mode);
		std::string modeName;
		if (modeIter == userModeCharToStr.end())
			modeName = std::string(mode);
		else
			modeName = modeIter->second;
		serverUserModes.insert(std::pair<ModeType, std::string> (currType, modeName));
		serverUserModeType.insert(std::pair<std::string, ModeType> (modeName, currType));
	}
}

void Protocol::parse005ChanTypes(const std::string& chanTypesChars) {
	channelTypes.clear();
	for (char chanType : chanTypesChars)
		channelTypes.insert(chanType);
}

void Protocol::parse005MaxModes(const std::string& modeCount) {
	std::istringstream modeStr (modeCount);
	modeStr >> maxModes;
	if (maxModes < 1)
		maxModes = 1;
}

void Protocol::parse005MaxTargets(const std::string& targCount) {
	std::istringstream targStr (targCount);
	targStr >> maxTargets;
	if (maxTargets < 1)
		maxTargets = 1;
}

void Protocol::handleData() {
	while (loaded) {
		if (receivedData.empty()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
			continue;
		}
		MutexLocker mutexLock (&processMutex);
		std::pair<std::string, std::unique_ptr<IRCMessage>> msgInfo (receivedData.front());
		receivedData.pop();
		std::string clientID (msgInfo.first);
		auto clientIter = clients.find(clientID);
		if (clientIter == clients.end())
			continue; // The client has since disconnected, so we'll just ignore everything it says
		std::unique_ptr<IRCMessage> msg (msgInfo.second);
		std::string command (msg->command());
		if (command == "001") {
			clientIter->second->markRegistered();
			callHook(HOOK_CLIENT_REGISTER_SELF, clientID);
		} else if (command == "004") {
			serverUserModes.clear();
			serverUserModeType.clear();
			for (char mode : msg->params()[3]) {
				if (userModeCharToStr.find(mode) == userModeCharToStr.end()) {
					serverUserModes.insert(std::pair<ModeType, std::string> (MODE_NOPARAM, std::string(mode)));
					serverUserModeType.insert(std::pair<std::string, ModeType> (std::string(mode), MODE_NOPARAM));
				} else {
					std::string modeName (userModeCharToStr.find(mode)->second);
					ModeType type = MODE_NOPARAM;
					if (modeName == "snomask")
						type = MODE_PARAM; // Unless 005 USERMODES (if it exists) says otherwise (and we'll always get it after 004), snomask is generally implemented as MODE_PARAM
					serverUserModes.insert(std::pair<ModeType, std::string> (type, modeName));
					serverUserModeType.insert(std::pair<std::string, ModeType> (modeName, type));
				}
			}
			callHook(HOOK_CLIENT_NUMERIC, clientID, "004", msg->params(), msg->tags());
		} else if (command == "005") {
			for (unsigned int i = 1; i < msg->params().size() - 1; i++) {
				const std::string currParam (msg->params()[i]);
				const size_t equalsPos = currParam.find('=');
				if (equalsPos == std::string::npos)
					continue;
				const std::string currType (currParam.substr(0, equalsPos));
				const std::string supportParam (currParam.substr(equalsPos + 1));
				if (currType == "PREFIX")
					parse005Prefix(supportParam);
				else if (currType == "CHANMODES")
					parse005ChanModes(supportParam);
				else if (currType == "USERMODES")
					parse005UserModes(supportParam);
				else if (currType == "CHANTYPES")
					parse005ChanTypes(supportParam);
				else if (currType == "MODES")
					parse005MaxModes(supportParam);
				else if (currType == "MAXTARGETS")
					parse005MaxTargets(supportParam);
			}
			callHook(HOOK_CLIENT_NUMERIC, "005", msg->params(), msg->tags());
		} else if (command == "221") {
			clientIter->second->clearModes();
			size_t currParam = 2;
			std::vector<std::string> params = msg->params();
			for (char mode : params[1]) {
				if (mode == '+')
					continue;
				std::string longmode;
				auto modeNameIter = userModeCharToStr.find(mode);
				if (modeNameIter == userModeCharToStr.end())
					longmode = std::string(mode);
				else
					longmode = modeNameIter->second;
				auto modeTypeIter = serverUserModeType.find(longmode);
				if (modeTypeIter == serverUserModeType.end() || modeTypeIter->second == MODE_NOPARAM)
					clientIter->second->setMode(longmode);
				else
					clientIter->second->setMode(longmode, params[currParam++]);
			}
			callHook(HOOK_CLIENT_NUMERIC, "221", msg->params(), msg->tags());
		} else if (command == "324") {
			
		} else if (command == "329") {
			
		} else if (command == "332") {
			
		} else if (command == "333") {
			
		} else if (command == "352") {
			
		} else if (command == "353") {
			
		} else if (command == "366") {
			
		} else if (command == "433") {
			
		} else if (command == "710") {
			
		} else if (command.size() == 3 && command[0] >= '0' && command[0] <= '9' && command[1] >= '0' && command[1] <= '9' && command[2] >= '0' && command[2] <= '9') {
			
		} else if (command == "CAP") {
			
		} else if (command == "PRIVMSG") {
			
		} else if (command == "NOTICE") {
			
		} else if (command == "MODE") {
			
		} else if (command == "TOPIC") {
			
		} else if (command == "NICK") {
			
		} else if (command == "JOIN") {
			
		} else if (command == "PART") {
			
		} else if (command == "QUIT") {
			
		} else if (command == "INVITE") {
			
		} else if (command == "AWAY") {
			
		} else {
			
		}
	}
}