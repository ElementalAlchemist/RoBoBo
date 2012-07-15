#include "protocol.h"

Protocol::Protocol(std::string server, std::map<std::string, std::string> conf, std::string workDir, bool dumpLogs, unsigned short debug, Base* botptr) : serverName(server), workingDir(workDir), config(conf), log(dumpLogs), debugLevel(debug), bot(botptr) {}

Protocol::~Protocol() {}

void Protocol::connectServer() {}

void Protocol::disconnectServer(std::string reason) {}

bool Protocol::isConnected() {
	return false;
}

bool Protocol::deadServer() {
	return true;
}

bool Protocol::isClient() {
	return false;
}

void Protocol::endDebug() {
	debugLevel = 0;
}

std::shared_ptr<Socket> Protocol::assignSocket(std::string socketType) {
	return bot->loadSocket(socketType);
}