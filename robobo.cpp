#include "robobo.h"

std::tr1::unordered_map<std::string, Server*> connectedServers;
std::tr1::unordered_map<std::string, Module*> loadedModules;
std::list<std::string> serverList;

inline void makeServerList(ConfigReader& config) {
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > serverConfig = config.getServerConfig();
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator serverIterator = serverConfig.begin(); serverIterator != serverConfig.end(); serverIterator++)
		serverList.insert(serverList.end(), serverIterator->first);
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
		Server* newConnection = new Server (serverIterator->first, thisServerConf, &modInterface);
		connectedServers.insert(std::pair<std::string, Server*> (serverIterator->first, newConnection));
	}
}

inline void loadModules(ConfigReader& config, ModuleInterface& modInterface) {
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > moduleConfig = config.getModConfig();
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator modIter = moduleConfig.begin(); modIter != moduleConfig.end(); modIter++) {
		std::string modName = modIter->first;
		std::string fileLoc = "modules/" + modName;
		void* openModule = dlopen(fileLoc.c_str(), RTLD_LAZY);
		if (openModule == NULL) {
			std::string error = "Could not load module " + modName + ": " + dlerror();
			std::perror(error.c_str());
			continue;
		}
		char* dlsymError;
		Module* spawnModule = (Module*) dlsym(openModule, "spawn");
		dlsymError = dlerror();
		if (dlsymError) {
			std::string error = "Could not load module " + modName + ": " + dlsymError;
			std::perror(error.c_str());
			continue;
		}
		
		Module* newModule = spawnModule;
		newModule->init(&loadedModules, &serverList, &modInterface);
		loadedModules.insert(std::pair<std::string, Module*> (modName, newModule));
	}
}

int main(int argc, char** argv) {
	ConfigReader config;
	ModuleInterface modInterface (&connectedServers, &loadedModules);
	makeServerList(config);
	connectServers(config, modInterface);
	loadModules(config, modInterface);
	pthread_exit(NULL);
}