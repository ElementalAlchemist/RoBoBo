#pragma once
#include "main.h"
#include <dlfcn.h>

enum LoadResult { LOAD_SUCCESS, LOAD_ALREADYLOADED, LOAD_ERROR, LOAD_INCOMPATIBLE, LOAD_FAILURE };

#include "socket.h"
#include "protocol.h"
#include "module.h"

typedef void* (module_spawn_t)(std::string, std::map<std::string, std::string>, std::string, unsigned short, Base*);

class Base {
	public:
		Base(std::string working, std::string config, std::string configFileName, unsigned short debug, bool log);
		void readConfiguration();
		void loadModules();
		void connectServers();
		void checkServers();
		void unloadEverything();
		// TODO: Module hooks
		// TODO: Protocol hooks
		LoadResult loadModule(std::string modName);
		void unloadModule(std::string modName, bool wasLoaded);
		void connectServer(std::string server);
		void disconnectServer(std::string server);
		Socket* loadSocket(std::string sockettype);
		void unloadSocket(std::string sockettype, Socket* socketptr);
	private:
		const std::string workingDir, configDir, configName;
		const unsigned short debugLevel;
		const bool logDump;
		bool startup;
		std::unordered_map<std::string, std::unordered_map<std::string, std::string>> serverConfig, moduleConfig;
		std::list<std::string> startupServers, startupModules;
		std::map<std::string, Protocol*> servers;
		std::map<std::string, Module*> highModules, mediumHighModules, normalModules, mediumLowModules, lowModules;
		std::unordered_map<std::string, void*> moduleFiles;
		std::map<std::string, Priority> modulePriority;
		std::map<std::string, std::list<std::string>> moduleServices, moduleSupports;
		std::map<std::string, std::string> moduleDescriptions;
		std::unordered_map<std::string, void*> socketFiles;
};