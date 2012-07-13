#include "module.h"

Module::Module(std::string modName, std::map<std::string, std::string> conf, std::string workDir, unsigned short debug, Base* botptr) : moduleName(modName), workingDir(workDir), config(conf), debugLevel(debug), bot(botptr) {}
// The default priority is PRI_NORMAL but can be overridden in the subclass's constructor

Module::~Module() {}
// This is just here because we need a virtual destructor.

Priority Module::priority() {
	return PRI_NORMAL;
}

bool Module::onLoadComplete() {
	return true;
}

void Module::onUnload() {}

void Module::onRehash() {}

void Module::onModuleLoad(std::string modName) {}

void Module::onModuleUnload(std::string modName) {}

bool Module::forceKeepAlive() {
	return false;
}

void Module::rehash(std::map<std::string, std::string> conf) {
	config = conf;
}

void Module::endDebug() {
	debugLevel = 0;
}

std::string Module::description() {
	return "A description has not been provided by the module author.";
}

std::list<std::string> Module::provides() {
	return std::list<std::string>();
}

std::list<std::string> Module::requires() {
	return std::list<std::string>();
}

std::list<std::string> Module::supports() {
	return std::list<std::string>();
}