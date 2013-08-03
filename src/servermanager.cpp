#include "servermanager.h"

void ServerManager::pointManagers(ModuleManager* mm, SocketManager* sm) {
	modmanager = mm;
	sockmanager = sm;
}

void ServerManager::connectStartupServers() {
	Config* config = Config::getHandle();
	std::list<std::unordered_map<std::string, std::string>> servers = config->getBlocks("server");
	for (auto serverConf : servers) {
		auto nameIter = serverConf.find("name");
		auto startupIter = serverConf.find("startup");
		if (nameIter != serverConf.end() && startupIter != serverConf.end() && Config::makeBool(startupIter->second))
			connectServer(nameIter->second);
	}
}

void ServerManager::connectServer(const std::string& server) {
	Config* config = Config::getHandle();
	std::unordered_map<std::string, std::string> serverConfig = config->getSingleBlockOnConditions("server", std::unordered_map<std::string, std::string> { { "name", server } });
	if (serverConfig.empty())
		throw ServerNotConfigured ();
	auto protocolIter = serverConfig.find("protocol");
	if (protocolIter == serverConfig.end())
		throw ServerNoProtocol ();
	void* protoFile = dlopen(("protocols/" + protocolIter->second + ".so").c_str(), RTLD_NOW);
	if (protoFile == nullptr)
		throw ProtoLoadFailed (dlerror());
	void* serverFunc = dlsym(protoFile, "serverType");
	if (serverFunc == nullptr) {
		const char* loadError = dlerror();
		if (loadError)
			throw ProtoLoadFailed (loadError);
		throw ProtoLoadFailed ("The serverType symbol was set to null, but it must be a valid function.");
	}
	bool (*serverCallFunc)() = (bool(*)()) serverFunc;
	bool isServer = serverCallFunc();
	void* spawnFunc = dlsym(protoFile, "spawn");
	if (spawnFunc == nullptr) {
		const char* loadError = dlerror();
		if (loadError)
			throw ProtoLoadFailed (loadError);
		throw ProtoLoadFailed ("The spawn symbol was set to null, but it must be a valid function.");
	}
	if (isServer) {
		ServerProtocol*(*spawnCallFunc)(const std::string&) = (ServerProtocol*(*)(const std::string&)) spawnFunc;
		std::shared_ptr<ServerProtocol> newProto (spawnCallFunc(server), std::bind(&ServerManager::unloadServerServer, this, server, protoFile, std::placeholders::_1));
		if (protoAPIVersions.find(newProto->apiVersion()) == protoAPIVersions.end())
			throw ProtoAPIMismatch ();
		newProto->pointManagers(modmanager, sockmanager);
		newProto->connectServer();
		serverServers.insert(std::pair<std::string, std::shared_ptr<ServerProtocol>> (protocolIter->second, newProto));
	} else {
		ClientProtocol*(*spawnCallFunc)(const std::string&) = (ClientProtocol*(*)(const std::string&)) spawnFunc;
		std::shared_ptr<ClientProtocol> newProto (spawnCallFunc(server), std::bind(&ServerManager::unloadClientServer, this, server, protoFile, std::placeholders::_1));
		if (protoAPIVersions.find(newProto->apiVersion()) == protoAPIVersions.end())
			throw ProtoAPIMismatch ();
		newProto->pointManagers(modmanager, sockmanager);
		newProto->connectServer();
		clientServers.insert(std::pair<std::string, std::shared_ptr<ClientProtocol>> (protocolIter->second, newProto));
	}
	serverTypes.insert(std::pair<std::string, std::string> (server, protocolIter->second));
	LogManager* logger = LogManager::getHandle();
	logger->log(LOG_DEBUG, "servers", "Server " + server + " loaded and connected.");
}

void ServerManager::disconnectServer(const std::string& server) {
	auto clientIter = clientServers.find(server);
	if (clientIter == clientServers.end()) {
		auto serverIter = serverServers.find(server);
		if (serverIter != serverServers.end()) {
			serverIter->second->disconnect();
			serverServers.erase(serverIter);
		}
	} else {
		clientIter->second->disconnect();
		clientServers.erase(clientIter);
	}
	LogManager* logger = LogManager::getHandle();
	logger->log(LOG_DEBUG, "servers", "Server " + server + " disconnected.");
}

size_t ServerManager::checkServers() {
	std::list<std::unordered_map<std::string, std::shared_ptr<ClientProtocol>>::iterator> clientRemove;
	std::list<std::unordered_map<std::string, std::shared_ptr<ServerProtocol>>::iterator> serverRemove;
	for (auto clientIter = clientServers.begin(); clientIter != clientServers.end(); ++clientIter) {
		if (!clientIter->second->connected() && clientIter->second->shouldUnload())
			clientRemove.push_back(clientIter);
	}
	for (auto serverIter = serverServers.begin(); serverIter != serverServers.end(); ++serverIter) {
		if (!serverIter->second->connected() && serverIter->second->shouldUnload())
			serverRemove.push_back(serverIter);
	}
	for (auto remove : clientRemove)
		clientServers.erase(remove);
	for (auto remove : serverRemove)
		serverServers.erase(remove);
	return clientServers.size() + serverServers.size();
}

void ServerManager::sendMsg(const std::string& server, const std::string& source, const std::string& target, const std::string& message, const std::map<std::string, std::string>& tags) {
	callEitherHook(&ClientProtocol::sendMsg, &ServerProtocol::sendMsg, server, std::forward<const std::string&>(source), std::forward<const std::string&>(target), std::forward<const std::string&>(message), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::sendNotice(const std::string& server, const std::string& source, const std::string& target, const std::string& message, const std::map<std::string, std::string>& tags) {
	callEitherHook(&ClientProtocol::sendNotice, &ServerProtocol::sendNotice, server, std::forward<const std::string&>(source), std::forward<const std::string&>(target), std::forward<const std::string&>(message), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::setMode(const std::string& server, const std::string& source, const std::string& target, const std::list<std::tuple<bool, std::string, std::string>>& modes, const std::map<std::string, std::string>& tags) {
	callEitherHook(&ClientProtocol::setMode, &ServerProtocol::setMode, server, std::forward<const std::string&>(source), std::forward<const std::string&>(target), std::forward<const std::list<std::tuple<bool, std::string, std::string>>&>(modes), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::joinChan(const std::string& server, const std::string& client, const std::string& channel, const std::map<std::string, std::string>& tags) {
	callServerHook(&ServerProtocol::joinChan, server, std::forward<const std::string&>(client), std::forward<const std::string&>(channel), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::joinChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& key, const std::map<std::string, std::string>& tags) {
	callClientHook(&ClientProtocol::joinChan, server, std::forward<const std::string&>(client), std::forward<const std::string&>(channel), std::forward<const std::string&>(key), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::joinChanOver(const std::string& server, const std::string& client, const std::string& channel, const std::list<std::string>& statuses, time_t timestamp, const std::map<std::string, std::string>& tags) {
	callServerHook(&ServerProtocol::joinChanOver, server, std::forward<const std::string&>(client), std::forward<const std::string&>(channel), std::forward<const std::list<std::string>&>(statuses), std::forward<time_t>(timestamp), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::partChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason, const std::map<std::string, std::string>& tags) {
	callEitherHook(&ClientProtocol::partChan, &ServerProtocol::partChan, server, std::forward<const std::string&>(client), std::forward<const std::string&>(channel), std::forward<const std::string&>(reason), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::kickUser(const std::string& server, const std::string& source, const std::string& channel, const std::string& user, const std::string& reason, const std::map<std::string, std::string>& tags) {
	callEitherHook(&ClientProtocol::kickUser, &ServerProtocol::kickUser, server, std::forward<const std::string&>(source), std::forward<const std::string&>(channel), std::forward<const std::string&>(user), std::forward<const std::string&>(reason), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::setTopic(const std::string& server, const std::string& source, const std::string& channel, const std::string& topic, const std::map<std::string, std::string>& tags) {
	callEitherHook(&ClientProtocol::setTopic, &ServerProtocol::setTopic, server, std::forward<const std::string&>(source), std::forward<const std::string&>(channel), std::forward<const std::string&>(topic), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::inviteUser(const std::string& server, const std::string& source, const std::string& channel, const std::string& user, const std::map<std::string, std::string>& tags) {
	callEitherHook(&ClientProtocol::inviteUser, &ServerProtocol::inviteUser, server, std::forward<const std::string&>(source), std::forward<const std::string&>(channel), std::forward<const std::string&>(user), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::knock(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason, const std::map<std::string, std::string>& tags) {
	callEitherHook(&ClientProtocol::knock, &ServerProtocol::knock, server, std::forward<const std::string&>(client), std::forward<const std::string&>(channel), std::forward<const std::string&>(reason), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::changeNick(const std::string& server, const std::string& client, const std::string& nick, const std::map<std::string, std::string>& tags) {
	callEitherHook(&ClientProtocol::changeNick, &ServerProtocol::changeNick, server, std::forward<const std::string&>(client), std::forward<const std::string&>(nick), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::sendPing(const std::string& server, const std::string& destServer, const std::map<std::string, std::string>& tags) {
	callServerHook(&ServerProtocol::sendPing, server, std::forward<const std::string&>(destServer), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::sendPing(const std::string& server, const std::string& client, const std::string& data, const std::map<std::string, std::string>& tags) {
	callClientHook(&ClientProtocol::sendPing, server, std::forward<const std::string&>(client), std::forward<const std::string&>(data), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::setAway(const std::string& server, const std::string& client, const std::string& reason, const std::map<std::string, std::string>& tags) {
	callEitherHook(&ClientProtocol::setAway, &ServerProtocol::setAway, server, std::forward<const std::string&>(client), std::forward<const std::string&>(reason), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::setUnaway(const std::string& server, const std::string& client, const std::map<std::string, std::string>& tags) {
	callEitherHook(&ClientProtocol::setUnaway, &ServerProtocol::setUnaway, server, std::forward<const std::string&>(client), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::oper(const std::string& server, const std::string& client, const std::string& username, const std::string& password, const std::map<std::string, std::string>& tags) {
	callClientHook(&ClientProtocol::oper, server, std::forward<const std::string&>(client), std::forward<const std::string&>(username), std::forward<const std::string&>(password), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::oper(const std::string& server, const std::string& client, const std::string& type, const std::map<std::string, std::string>& tags) {
	callServerHook(&ServerProtocol::oper, server, std::forward<const std::string&>(client), std::forward<const std::string&>(type), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::sendServerNotice(const std::string& server, const std::string& type, const std::string& message, const std::map<std::string, std::string>& tags) {
	callServerHook(&ServerProtocol::sendServerNotice, server, std::forward<const std::string&>(type), std::forward<const std::string&>(message), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::setMetadata(const std::string& server, const std::string& target, const std::string& key, const std::string& value, const std::map<std::string, std::string>& tags) {
	callServerHook(&ServerProtocol::setMetadata, server, std::forward<const std::string&>(target), std::forward<const std::string&>(key), std::forward<const std::string&>(value), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::setXLine(const std::string& server, const std::string& source, const std::string& lineType, const std::string& mask, time_t duration, const std::string& reason, const std::map<std::string, std::string>& tags) {
	callServerHook(&ServerProtocol::setXLine, server, std::forward<const std::string&>(source), std::forward<const std::string&>(lineType), std::forward<const std::string&>(mask), std::forward<time_t>(duration), std::forward<const std::string&>(reason), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::removeXLine(const std::string& server, const std::string& source, const std::string& lineType, const std::string& mask, const std::map<std::string, std::string>& tags) {
	callServerHook(&ServerProtocol::removeXLine, server, std::forward<const std::string&>(source), std::forward<const std::string&>(lineType), std::forward<const std::string&>(mask), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::changeIdent(const std::string& server, const std::string& user, const std::string& ident, const std::map<std::string, std::string>& tags) {
	callServerHook(&ServerProtocol::changeIdent, server, std::forward<const std::string&>(user), std::forward<const std::string&>(ident), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::changeHost(const std::string& server, const std::string& user, const std::string& host, const std::map<std::string, std::string>& tags) {
	callServerHook(&ServerProtocol::changeHost, server, std::forward<const std::string&>(user), std::forward<const std::string&>(host), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::changeGecos(const std::string& server, const std::string& user, const std::string& gecos, const std::map<std::string, std::string>& tags) {
	callServerHook(&ServerProtocol::changeGecos, server, std::forward<const std::string&>(user), std::forward<const std::string&>(gecos), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::sendWallops(const std::string& server, const std::string& source, const std::string& message, const std::map<std::string, std::string>& tags) {
	callEitherHook(&ClientProtocol::sendWallops, &ServerProtocol::sendWallops, server, std::forward<const std::string&>(source), std::forward<const std::string&>(message), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::pushLine(const std::string& server, const std::string& user, const IRCMessage* line) {
	callServerHook(&ServerProtocol::pushLine, server, std::forward<const std::string&>(user), std::forward<const IRCMessage*>(line));
}

void ServerManager::sendOtherData(const std::string& server, const std::string& client, const IRCMessage* line) {
	callClientHook(&ClientProtocol::sendOtherData, server, std::forward<const std::string&>(client), std::forward<const IRCMessage*>(line));
}

void ServerManager::sendOtherData(const std::string& server, const IRCMessage* line) {
	callServerHook(&ServerProtocol::sendOtherData, server, std::forward<const IRCMessage*>(line));
}

std::string ServerManager::addClient(const std::string& server, const std::string& nick, const std::string& ident, const std::string& host, const std::string& gecos, const std::map<std::string, std::string>& tags) {
	return callServerHook(&ServerProtocol::addClient, server, std::forward<const std::string&>(nick), std::forward<const std::string&>(ident), std::forward<const std::string&>(host), std::forward<const std::string&>(gecos), std::forward<const std::map<std::string, std::string>&>(tags));
}

std::string ServerManager::addClient(const std::string& server, const std::string& nick, const std::string& ident, const std::string& gecos) {
	return callClientHook(&ClientProtocol::addClient, server, std::forward<const std::string&>(nick), std::forward<const std::string&>(ident), std::forward<const std::string&>(gecos));
}

void ServerManager::removeClient(const std::string& server, const std::string& client, const std::map<std::string, std::string>& tags) {
	callServerHook(&ServerProtocol::removeClient, server, std::forward<const std::string&>(client), std::forward<const std::map<std::string, std::string>&>(tags));
}

void ServerManager::removeClient(const std::string& server, const std::string& client) {
	callClientHook(&ClientProtocol::removeClient, server, std::forward<const std::string&>(client));
}

std::list<std::pair<std::string, bool>> ServerManager::activeServers(bool requestedByServerModule) {
	std::list<std::pair<std::string, bool>> serverList;
	for (auto client : clientServers)
		serverList.push_back(std::pair<std::string, bool> (client.first, !requestedByServerModule));
	for (auto server : serverServers)
		serverList.push_back(std::pair<std::string, bool> (server.first, requestedByServerModule));
	return serverList;
}

std::list<std::pair<std::string, bool>> ServerManager::connectedServers(bool requestedByServerModule) {
	std::list<std::pair<std::string, bool>> serverList;
	for (auto client : clientServers) {
		if (client.second->connected())
			serverList.push_back(std::pair<std::string, bool> (client.first, !requestedByServerModule));
	}
	for (auto server : serverServers) {
		if (server.second->connected())
			serverList.push_back(std::pair<std::string, bool> (server.first, requestedByServerModule));
	}
	return serverList;
}

std::string ServerManager::serverType(const std::string& server) {
	auto serverIter = serverTypes.find(server);
	if (serverIter == serverTypes.end())
		throw ServerNotLoaded ();
	return serverIter->second;
}

std::list<std::string> ServerManager::networkServerList(const std::string& server) {
	return callServerHook(&ServerProtocol::networkServerList, server);
}

std::list<std::string> ServerManager::xLineTypes(const std::string& server) {
	return callServerHook(&ServerProtocol::xLineTypes, server);
}

std::list<std::string> ServerManager::xLineList(const std::string& server, const std::string& lineType) {
	return callServerHook(&ServerProtocol::xLineList, server, std::forward<const std::string&>(lineType));
}

time_t ServerManager::xLineExpiry(const std::string& server, const std::string& lineType, const std::string& mask) {
	return callServerHook(&ServerProtocol::xLineExpiry, server, std::forward<const std::string&>(lineType), std::forward<const std::string&>(mask));
}

std::string ServerManager::xLineReason(const std::string& server, const std::string& lineType, const std::string& mask) {
	return callServerHook(&ServerProtocol::xLineReason, server, std::forward<const std::string&>(lineType), std::forward<const std::string&>(mask));
}

std::set<std::string> ServerManager::chanTypes(const std::string& server) {
	return callEitherHook(&ClientProtocol::chanTypes, &ServerProtocol::chanTypes, server);
}

std::list<std::pair<ModeType, std::string>> ServerManager::allChanModes(const std::string& server) {
	return callEitherHook(&ClientProtocol::allChanModes, &ServerProtocol::allChanModes, server);
}

char ServerManager::prefixSymbol(const std::string& server, const std::string& mode) {
	return callEitherHook(&ClientProtocol::prefixSymbol, &ServerProtocol::prefixSymbol, server, std::forward<const std::string&>(mode));
}

std::pair<std::string, char> ServerManager::compareStatus(const std::string& server, const std::string& status0, const std::string& status1) {
	std::pair<std::string, char>(ClientProtocol::*clientFunc)(const std::string&, const std::string&) = &ClientProtocol::compareStatus;
	std::pair<std::string, char>(ServerProtocol::*serverFunc)(const std::string&, const std::string&) = &ServerProtocol::compareStatus;
	return callEitherHook(clientFunc, serverFunc, server, std::forward<const std::string&>(status0), std::forward<const std::string&>(status1));
}

std::pair<std::string, char> ServerManager::compareStatus(const std::string& server, const std::string& status0, char status1) {
	std::pair<std::string, char>(ClientProtocol::*clientFunc)(const std::string&, char) = &ClientProtocol::compareStatus;
	std::pair<std::string, char>(ServerProtocol::*serverFunc)(const std::string&, char) = &ServerProtocol::compareStatus;
	return callEitherHook(clientFunc, serverFunc, server, std::forward<const std::string&>(status0), std::forward<char>(status1));
}

std::pair<std::string, char> ServerManager::compareStatus(const std::string& server, char status0, char status1) {
	std::pair<std::string, char>(ClientProtocol::*clientFunc)(char, char) = &ClientProtocol::compareStatus;
	std::pair<std::string, char>(ServerProtocol::*serverFunc)(char, char) = &ServerProtocol::compareStatus;
	return callEitherHook(clientFunc, serverFunc, server, std::forward<char>(status0), std::forward<char>(status1));
}

std::list<std::string> ServerManager::chanList(const std::string& server) {
	return callServerHook(&ServerProtocol::chanList, server);
}

std::string ServerManager::chanTopic(const std::string& server, const std::string& channel) {
	return callEitherHook(&ClientProtocol::chanTopic, &ServerProtocol::chanTopic, server, std::forward<const std::string&>(channel));
}

std::string ServerManager::chanTopicSetter(const std::string& server, const std::string& channel) {
	return callEitherHook(&ClientProtocol::chanTopicSetter, &ServerProtocol::chanTopicSetter, server, std::forward<const std::string&>(channel));
}

time_t ServerManager::chanTimestamp(const std::string& server, const std::string& channel) {
	return callEitherHook(&ClientProtocol::chanTimestamp, &ServerProtocol::chanTimestamp, server, std::forward<const std::string&>(channel));
}

std::list<std::string> ServerManager::chanUsers(const std::string& server, const std::string& channel) {
	return callEitherHook(&ClientProtocol::chanUsers, &ServerProtocol::chanUsers, server, std::forward<const std::string&>(channel));
}

bool ServerManager::userInChan(const std::string& server, const std::string& channel, const std::string& user) {
	return callEitherHook(&ClientProtocol::userInChan, &ServerProtocol::userInChan, server, std::forward<const std::string&>(channel), std::forward<const std::string&>(user));
}

std::pair<std::string, char> ServerManager::userStatus(const std::string& server, const std::string& channel, const std::string& user) {
	return callEitherHook(&ClientProtocol::userStatus, &ServerProtocol::userStatus, server, std::forward<const std::string&>(channel), std::forward<const std::string&>(user));
}

bool ServerManager::userHasStatus(const std::string& server, const std::string& channel, const std::string& user, const std::string& status) {
	bool(ClientProtocol::*clientFunc)(const std::string&, const std::string&, const std::string&) = &ClientProtocol::userHasStatus;
	bool(ServerProtocol::*serverFunc)(const std::string&, const std::string&, const std::string&) = &ServerProtocol::userHasStatus;
	return callEitherHook(clientFunc, serverFunc, server, std::forward<const std::string&>(channel), std::forward<const std::string&>(user), std::forward<const std::string&>(status));
}

bool ServerManager::userHasStatus(const std::string& server, const std::string& channel, const std::string& user, char status) {
	bool(ClientProtocol::*clientFunc)(const std::string&, const std::string&, char) = &ClientProtocol::userHasStatus;
	bool(ServerProtocol::*serverFunc)(const std::string&, const std::string&, char) = &ServerProtocol::userHasStatus;
	return callEitherHook(clientFunc, serverFunc, server, std::forward<const std::string&>(channel), std::forward<const std::string&>(user), std::forward<char>(status));
}

bool ServerManager::userHasStatusOrGreater(const std::string& server, const std::string& channel, const std::string& user, const std::string& status) {
	bool(ClientProtocol::*clientFunc)(const std::string&, const std::string&, const std::string&) = &ClientProtocol::userHasStatusOrGreater;
	bool(ServerProtocol::*serverFunc)(const std::string&, const std::string&, const std::string&) = &ServerProtocol::userHasStatusOrGreater;
	return callEitherHook(clientFunc, serverFunc, server, std::forward<const std::string&>(channel), std::forward<const std::string&>(user), std::forward<const std::string&>(status));
}

bool ServerManager::userHasStatusOrGreater(const std::string& server, const std::string& channel, const std::string& user, char status) {
	bool(ClientProtocol::*clientFunc)(const std::string&, const std::string&, char) = &ClientProtocol::userHasStatusOrGreater;
	bool(ServerProtocol::*serverFunc)(const std::string&, const std::string&, char) = &ServerProtocol::userHasStatusOrGreater;
	return callEitherHook(clientFunc, serverFunc, server, std::forward<const std::string&>(channel), std::forward<const std::string&>(user), std::forward<char>(status));
}

std::map<std::string, std::string> ServerManager::chanModes(const std::string& server, const std::string& channel) {
	return callEitherHook(&ClientProtocol::chanModes, &ServerProtocol::chanModes, server, std::forward<const std::string&>(channel));
}

bool ServerManager::chanHasMode(const std::string& server, const std::string& channel, const std::string& mode) {
	return callEitherHook(&ClientProtocol::chanHasMode, &ServerProtocol::chanHasMode, server, std::forward<const std::string&>(channel), std::forward<const std::string&>(mode));
}

std::string ServerManager::chanModeParam(const std::string& server, const std::string& channel, const std::string& mode) {
	return callEitherHook(&ClientProtocol::chanModeParam, &ServerProtocol::chanModeParam, server, std::forward<const std::string&>(channel), std::forward<const std::string&>(mode));
}

std::list<std::string> ServerManager::chanListModeList(const std::string& server, const std::string& channel, const std::string& listMode) {
	return callEitherHook(&ClientProtocol::chanListModeList, &ServerProtocol::chanListModeList, server, std::forward<const std::string&>(channel), std::forward<const std::string&>(listMode));
}

std::list<std::string> ServerManager::clientList(const std::string& server) {
	return callEitherHook(&ClientProtocol::clientList, &ServerProtocol::clientList, server);
}

std::string ServerManager::userNick(const std::string& server, const std::string& user) {
	return callEitherHook(&ClientProtocol::userNick, &ServerProtocol::userNick, server, std::forward<const std::string&>(user));
}

std::string ServerManager::userIdent(const std::string& server, const std::string& user) {
	return callEitherHook(&ClientProtocol::userIdent, &ServerProtocol::userIdent, server, std::forward<const std::string&>(user));
}

std::string ServerManager::userHost(const std::string& server, const std::string& user) {
	return callEitherHook(&ClientProtocol::userHost, &ServerProtocol::userHost, server, std::forward<const std::string&>(user));
}

std::string ServerManager::userGecos(const std::string& server, const std::string& user) {
	return callEitherHook(&ClientProtocol::userGecos, &ServerProtocol::userGecos, server, std::forward<const std::string&>(user));
}

std::map<std::string, std::string> ServerManager::userModes(const std::string& server, const std::string& user) {
	return callEitherHook(&ClientProtocol::userModes, &ServerProtocol::userModes, server, std::forward<const std::string&>(user));
}

bool ServerManager::userHasMode(const std::string& server, const std::string& user, const std::string& mode) {
	return callEitherHook(&ClientProtocol::userHasMode, &ServerProtocol::userHasMode, server, std::forward<const std::string&>(user), std::forward<const std::string&>(mode));
}

std::string ServerManager::userModeParam(const std::string& server, const std::string& user, const std::string& mode) {
	return callEitherHook(&ClientProtocol::userHasMode, &ServerProtocol::userHasMode, server, std::forward<const std::string&>(user), std::forward<const std::string&>(mode));
}

std::list<std::string> ServerManager::userListModeList(const std::string& server, const std::string& user, const std::string& listMode) {
	return callEitherHook(&ClientProtocol::userListModeList, &ServerProtocol::userListModeList, server, std::forward<const std::string&>(user), std::forward<const std::string&>(listMode));
}

std::set<std::string> ServerManager::userChans(const std::string& server, const std::string& user) {
	return callEitherHook(&ClientProtocol::userChans, &ServerProtocol::userChans, server, std::forward<const std::string&>(user));
}

time_t ServerManager::userTimestamp(const std::string& server, const std::string& user) {
	return callServerHook(&ServerProtocol::userTimestamp, server, std::forward<const std::string&>(user));
}

time_t ServerManager::userNickTimestamp(const std::string& server, const std::string& user) {
	return callServerHook(&ServerProtocol::userNickTimestamp, server, std::forward<const std::string&>(user));
}

time_t ServerManager::userAwayTimestamp(const std::string& server, const std::string& user) {
	return callServerHook(&ServerProtocol::userAwayTimestamp, server, std::forward<const std::string&>(user));
}

void ServerManager::unloadClientServer(const std::string& name, void* protoFile, ClientProtocol* serverPtr) {
	delete serverPtr;
	dlclose(protoFile);
	serverTypes.erase(name);
	LogManager* logger = LogManager::getHandle();
	logger->log(LOG_DEBUG, "servers", "Server " + name + " unloaded.");
}

void ServerManager::unloadServerServer(const std::string& name, void* protoFile, ServerProtocol* serverPtr) {
	delete serverPtr;
	dlclose(protoFile);
	serverTypes.erase(name);
	LogManager* logger = LogManager::getHandle();
	logger->log(LOG_DEBUG, "servers", "Server " + name + " unloaded.");
}

template<typename RetVal, typename... Args>
RetVal ServerManager::callEitherHook(RetVal(ClientProtocol::*clientFunc)(Args...), RetVal(ServerProtocol::*serverFunc)(Args...), const std::string& server, Args&&... args) {
	auto clientIter = clientServers.find(server);
	if (clientIter == clientServers.end()) {
		auto serverIter = serverServers.find(server);
		if (serverIter == serverServers.end())
			throw ServerNotLoaded ();
		return ((*serverIter->second).*(serverFunc))(args...);
	}
	return ((*clientIter->second).*(clientFunc))(args...);
}

template<typename RetVal, typename... Args>
RetVal ServerManager::callClientHook(RetVal(ClientProtocol::*func)(Args...), const std::string& server, Args&&... args) {
	auto clientIter = clientServers.find(server);
	if (clientIter == clientServers.end()) {
		if (serverServers.find(server) == serverServers.end())
			throw ServerNotLoaded ();
		throw ServerDoesNotSupport ();
	}
	return ((*clientIter->second).*(func))(args...);
}

template<typename RetVal, typename... Args>
RetVal ServerManager::callServerHook(RetVal(ServerProtocol::*func)(Args...), const std::string& server, Args&&... args) {
	auto serverIter = serverServers.find(server);
	if (serverIter == serverServers.end()) {
		if (clientServers.find(server) == clientServers.end())
			throw ServerNotLoaded ();
		throw ServerDoesNotSupport ();
	}
	return ((*serverIter->second).*(func))(args...);
}