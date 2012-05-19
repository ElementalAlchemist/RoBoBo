#include "base.h"

Base::Base(std::string working, std::string config, std::string configFileName, unsigned short debug, bool log) : workingDir(working), configDir(config), configName(configFileName), debugLevel(debug), logDump(log), startup(true) {}

void Base::readConfiguration() {
	// TODO: parse config file
}

void Base::startQueueThread() {
	queueThread = std::thread(&Base::messageQueue);
}

void Base::loadModules() {
	for (std::string modName : startupModules)
		loadModule(modName);
}

void Base::connectServers() {
	for (std::string serverName : startupServers)
		connectServer(serverName);
}

void Base::checkModules() {
	// TODO: module and server check
}

void Base::unloadEverything() {
	for (std::pair<std::string, Protocol*> server : servers)
		disconnectServer(server.first);
	for (std::pair<std::string, Module*> module : modules)
		unloadModule(module.first);
}

LoadResult Base::loadModule(std::string modName) {
	// TODO: load modules
	return LOAD_SUCCESS;
}

void Base::unloadModule(std::string modName) {
	// TODO: also unload modules
}

void Base::connectServer(std::string server) {
	// TODO: basically everything
}

void Base::disconnectServer(std::string server) {
	// TODO: pretty much, what did you expect?
}

Socket* Base::loadSocket(std::string sockettype) {
	// TODO: load and return a socket
	return NULL;
}

void Base::messageQueue() {
	// TODO: process the queue once I get a better-defined list of types
}