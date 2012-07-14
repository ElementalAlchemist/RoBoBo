#pragma once
#include "main.h"
#include <dlfcn.h>

/** Load result flags
 * A set of flags, one of which is returned by loadModule to show either that the module was loaded successfully
 * or why the module could not be loaded.
 */

#include "socket.h"
#include "protocol.h"
#include "module.h"

// The spawn function needs a typedef so that it can be called after a dlsym.
// This typedef is used to cast the result of dlsym so that it can be used. (see Base::loadModule)
typedef void* (module_spawn_t)(std::string, std::map<std::string, std::string>, std::string, unsigned short, Base*);
typedef void* (protocol_spawn_t)(std::string, std::map<std::string, std::string>, std::string, bool, unsigned short, Base*);
typedef void* (socket_spawn_t)();

class Base {
	public:
		Base(std::string working, std::string config, std::string configFileName, unsigned short debug, bool log);
		void readConfiguration();
		void loadModules();
		bool completeStartupLoad(std::string modName, Module* modptr);
		void connectServers();
		void checkServers();
		void unloadEverything();
		LoadResult loadModule(std::string modName);
		void unloadModule(std::string modName, bool wasLoaded);
		void connectServer(std::string server);
		void disconnectServer(std::string server, std::string reason = "");
		Socket* loadSocket(std::string sockettype);
		void unloadSocket(std::string sockettype, Socket* socketptr);
		void rehash();
		void endDebug();
		
		
		
		std::list<std::string> moduleList();
		std::list<std::string> providedServices();
		bool serviceIsProvided(std::string service);
		std::list<std::string> serviceProviders(std::string service);
		std::list<std::string> serviceUsers(std::string service);
		void refreshServices(std::string module);
	private:
		const std::string workingDir, configDir, configName;
		const unsigned short debugLevel;
		const bool logDump;
		bool startup;
		std::map<std::string, std::map<std::string, std::string>> serverConfig, moduleConfig;
		std::list<std::string> startupServers, startupModules;
		std::map<std::string, Protocol*> servers;
		std::map<std::string, Module*> highModules, mediumHighModules, normalModules, mediumLowModules, lowModules;
		std::unordered_map<std::string, void*> moduleFiles;
		std::map<std::string, Priority> modulePriority;
		std::map<std::string, std::list<std::string>> moduleServices, moduleRequires, moduleSupports;
		std::map<std::string, std::string> moduleDescriptions;
		std::unordered_map<std::string, void*> protocolFiles;
		std::unordered_map<std::string, std::set<std::string>> protocolTypes;
		std::unordered_map<std::string, void*> socketFiles;
		std::unordered_map<std::string, size_t> socketCounts;
		std::mutex modHookMutex;
		
		// A manager class to automatically release mutexes makes mutex handling easier and more reliable
		class MutexManager {
			public:
				MutexManager(std::mutex* mutexPtr);
				~MutexManager();
				MutexManager(const MutexManager&) = delete;
				MutexManager& operator=(MutexManager const&) = delete;
			private:
				std::mutex* mutex;
		};
};