#include "protocol.h"

Protocol::Protocol() : floodThrottle(true) {}

void Protocol::connectServer() {
	
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