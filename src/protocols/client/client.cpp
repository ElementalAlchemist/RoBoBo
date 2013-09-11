#include "client.h"

Client::Client(std::string&& id, std::string&& nick, std::string&& ident, std::string&& gecos, const Protocol* mod)
	: User(std::forward<std::string> (id), std::forward<std::string> (nick), std::forward<std::string> (ident), std::forward<std::string> (gecos)),
	expectingReconnect(true), proto(mod) {}

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
	
}

void Client::sendQueue() {
	
}

void Client::decrementSeconds() {
	while (socket->isConnected()) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		MutexLocker mutexLock (&sendMutex);
		if (penaltySeconds > 0)
			penaltySeconds--;
	}
	penaltySeconds = 0;
}