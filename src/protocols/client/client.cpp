#include "client.h"

Client::Client(std::string&& id, std::string&& nick, std::string&& ident, std::string&& gecos, const Protocol* mod)
	: User(std::forward<std::string> (id), std::forward<std::string> (nick), std::forward<std::string> (ident), std::forward<std::string> (gecos)),
	expectingReconnect(true), proto(mod) {}
	// TODO: populate commandPenalty

Client::~Client() {
	
}

void Client::disconnect() {
	// TODO
	expectingReconnect = false;
}

bool Client::checkConnection() const {
	if (socket)
		return socket->isConnected();
	return false;
}

bool Client::wantsToReconnect() const {
	return expectingReconnect;
}

void Client::doReconnect() {
	
}

std::map<std::string, std::string> Client::modes() const {
	return clientModes;
}

bool Client::modeSet(const std::string& mode) const {
	return clientModes.find(mode) != clientModes.end();
}

std::string Client::modeParam(const std::string& mode) const {
	auto modeIter = clientModes.find(mode);
	if (modeIter == clientModes.end())
		return "";
	return modeIter->second;
}

std::list<std::string> Client::listModeList(const std::string& mode) const {
	auto listModeIter = clientListModes.find(mode);
	if (listModeIter == clientListModes.end())
		return std::list<std::string> ();
	return listModeIter->second;
}

bool Client::itemInList(const std::string& mode, const std::string& param) const {
	auto listModeIter = clientListModes.find(mode);
	if (listModeIter == clientListModes.end())
		return false;
	auto listIter = std::find(listModeIter->second.begin(), listModeIter->second.end(), param);
	return listIter != listModeIter->second.end();
}

void Client::setMode(const std::string& mode) {
	clientModes.insert(std::pair<std::string, std::string> (mode, ""));
}

void Client::setMode(const std::string& mode, const std::string& param) {
	clientModes[mode] = param; // If the mode is already set, we should change its param, but if not, it should be added
}

void Client::unsetMode(const std::string& mode) {
	clientModes.erase(mode);
}

void Client::setListMode(const std::string& mode, const std::string& param) {
	auto listModeIter = clientListModes.find(mode);
	if (listModeIter == clientListModes.end()) {
		clientListModes[mode].push_back(param);
		return;
	}
	auto listIter = std::find(listModeIter->second.begin(), listModeIter->second.end(), param);
	if (listIter == listModeIter->second.end())
		listModeIter->second.push_back(param);
}

void Client::unsetListMode(const std::string& mode, const std::string& param) {
	auto listModeIter = clientListModes.find(mode);
	if (listModeIter == clientListModes.end())
		return;
	listModeIter->second.remove(param);
	if (listModeIter->second.empty())
		clientListModes.erase(listModeIter);
}

void Client::sendLine(const IRCMessage* line) {
	socket->sendData(line->rawLine());
}

void Client::receiveData() {
	LogManager* logger = LogManager::getHandle();
	while (socket->isConnected()) {
		std::string newMsg;
		try {
			newMsg = socket->receive();
		} catch (const SocketOperationFailed& ex) {
			logger->log(LOG_DEFAULT, "protocol-client", "Connection failed for client " + userID + " (" + userNick + "!" + userIdent + "@" + userHost + " on server " + proto->servName() + ") during receive.");
			break;
		}
		proto->processIncoming(userID, IRCMessage (newMsg));
		logger->log(LOG_ALL, "protocol-client-recv-" + proto->servName(), newMsg);
	}
}

void Client::sendQueue() {
	while (socket->isConnected()) {
		if (linesToSend.empty()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(25));
			continue;
		}
		std::unique_ptr<IRCMessage> sendingLine = linesToSend.front();
		linesToSend.pop();
		unsigned int thisPenalty = 1;
		auto penaltyIter = commandPenalty.find(sendingLine->command());
		if (penaltyIter != commandPenalty.end())
			thisPenalty = penaltyIter->second;
		while (penaltySeconds > 10)
			std::this_thread::sleep_for(std::chrono::seconds(1));
		MutexLocker mutexLock (&sendMutex);
		penaltySeconds += thisPenalty;
		if (socket->isConnected()) // to make sure the connection didn't get lost during the wait
			socket->sendData(sendingLine->rawLine());
	}
	if (socket->isConnected()) {
		MutexLocker mutexLock (&sendMutex);
		while (!linesToSend.empty()) {
			socket->sendData(linesToSend.front()->rawLine());
			linesToSend.pop();
		}
	}
}

void Client::decrementSeconds() {
	LogManager* logger = LogManager::getHandle();
	while (socket->isConnected()) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		MutexLocker mutexLock (&sendMutex);
		if (penaltySeconds > 0) {
			penaltySeconds--;
			std::ostringstream logMsg;
			logMsg << "Penalty second count reduced to " << penaltySeconds;
			logger->log(LOG_ALL, "protocol-client-penalty-" + proto->servName(), logMsg.str());
		}
	}
	MutexLocker mutexLock (&sendMutex);
	penaltySeconds = 0;
}