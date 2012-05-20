#include "module.h"

Module::Module(std::string modName, std::map<std::string, std::string> conf, std::string workDir, unsigned short debug, Base* botptr) : moduleName(modName), config(conf), workingDir(workDir), debugLevel(debug), bot(botptr), priority(PRI_NORMAL) {}
// The default priority is PRI_NORMAL but can be overridden in the subclass's constructor

bool Module::onLoadComplete() {
	return true;
}

void Module::onRehash() {}

void Module::onModuleLoad(std::string modName) {}

void Module::onModuleUnload(std::string modName) {}

std::string Module::description() {
	return "A description has not been provided.";
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