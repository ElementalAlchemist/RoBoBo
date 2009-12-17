#include "robobo.h"

std::tr1::unordered_map<std::string, Server> connectedServers;
std::tr1::unordered_map<std::string, Module> loadedModules;
std::list<std::string> serverList, moduleList;

inline void makeServerList(ConfigReader& config) {
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > serverConfig = config.getServerConfig();
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator serverIterator = serverConfig.begin(); serverIterator != serverConfig.end(); serverIterator++)
		serverList.insert(serverList.end(), serverIterator->first);
}

inline void makeModuleList(ConfigReader& config) {
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > modConfig = config.getModConfig();
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator modIterator = modConfig.begin(); modIterator != modConfig.end(); modIterator++) {
		moduleList.insert(moduleList.end(), modIterator->first);
	}
}

void connectServers(ConfigReader& config, ModuleInterface& modInterface) {
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > serverConfig = config.getServerConfig();
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator serverIterator = serverConfig.begin(); serverIterator != serverConfig.end(); serverIterator++) {
		std::tr1::unordered_map<std::string, std::string> thisServerConf;
		for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator confIter = serverConfig.begin(); confIter != serverConfig.end(); confIter++) {
			if (confIter->first == serverIterator->first) {
				thisServerConf = confIter->second;
				break;
			}
		}
		connectedServers.insert(std::pair<std::string, Server> (serverIterator->first, Server (serverIterator->first, thisServerConf, &modInterface)));
	}
}

/* void loadModules(ConfigReader& config, ModuleInterface& modInterface) {
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
		moduleList.insert(std::pair<std::string, Module> (modName, *newModule, &modInterface));
	}
} */

int main(int argc, char** argv) {
	ConfigReader config;
	ModuleInterface modInterface (&connectedServers, &loadedModules);
	makeServerList(config);
	makeModuleList(config);
	connectServers(config, modInterface);
	//loadModules(config, modInterface);
}