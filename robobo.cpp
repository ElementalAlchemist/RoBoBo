#include "robobo.h"

std::tr1::unordered_map<std::string, Server> connectedServers;
std::tr1::unordered_map<std::string, Module> loadedModules;
std::list<std::string> serverList, moduleList;

void makeServerList(ConfigReader& config) {
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > serverConfig = config.getServerConfig();
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator serverIterator = serverConfig.begin(); serverIterator != serverConfig.end(); serverIterator++) {
		serverList.insert(serverList.end(), serverIterator->first);
	}
}

void makeModuleList(ConfigReader& config) {
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > modConfig = config.getModConfig();
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator modIterator = modConfig.begin(); modIterator != modConfig.end(); modIterator++) {
		moduleList.insert(moduleList.end(), modIterator->first);
	}
}

void connectServers(ConfigReader& config) {
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > serverConfig = config.getServerConfig();
	for (std::list<std::string>::iterator serverIterator = serverList.begin(); serverIterator != serverList.end(); serverIterator++) {
		connectedServers.insert(std::pair<std::string, Server> (*serverIterator, Server (*serverIterator, serverConfig[*serverIterator], moduleList)));
	}
}

/* void loadModules(ConfigReader& config) {
	for (std::list<std::string>::iterator modListIter = moduleList.begin(); modListIter != moduleList.end(); modListIter++) {
		std::string modName = *modListIter;
		std::string fileLoc = "modules/" + modName;
		void* openModule = dlopen(fileLoc.c_str(), RTLD_LAZY);
		if (openModule == NULL) {
			std::string error = "Could not load module: " + modName + ": " + dlerror();
			std::perror(error.c_str());
			continue;
		}
		char* dlsymError;
		void* spawnModule = dlsym(openModule, "spawn");
		dlsymError = dlerror();
		if (dlsymError) {
			std::string error = "Could not load module: " + modName + ": " + dlsymError;
			std::perror(error.c_str());
			continue;
		}
		void* unspawnModule = dlsym(openModule, "unspawn");
		dlsymError = dlerror();
		if (dlsymError) {
			std::string error = "Could not load module: " + modName + ": " + dlsymError;
			std::perror(error.c_str());
			continue;
		} // this should exist but we don't use it yet
		
		Module* newModule = spawnModule();
		moduleList.insert(std::pair<std::string, Module> (modName, *newModule));
	}
} */

int main(int argc, char** argv) {
	ConfigReader config;
	makeModuleList(config);
	makeServerList(config);
	connectServers(config);
	//loadModules(config);
}