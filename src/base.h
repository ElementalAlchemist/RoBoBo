#include "main.h"

enum LoadResult { LOAD_SUCCESS, LOAD_NOEXIST, LOAD_ALREADYLOADED };

class Base {
	public:
		Base(std::string working, std::string config, std::string configFileName, unsigned short debug, bool log);
		~Base();
		void readConfiguration();
		void startQueueThread();
		void loadModules();
		void connectServers();
		void checkModules();
		void unloadEverything();
		// TODO: Module hooks
		// TODO: Protocol hooks
	private:
		const std::string workingDir, configDir, configName;
		const unsigned short debugLevel;
		const bool logDump;
		std::unordered_map<std::string, std::unordered_map<std::string, std::string>> serverConfig, moduleConfig;
		std::list<std::string> startupServers;
		std::map<std::string, Protocol*> servers;
		std::map<std::string, Module*> modules;
		std::unordered_map<std::string, void*> moduleFiles;
		// TODO: everything
};