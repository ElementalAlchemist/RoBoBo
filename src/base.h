#include "main.h"

enum LoadResult { LOAD_SUCCESS, LOAD_NOEXIST, LOAD_ALREADYLOADED };
enum MsgType { MSG_PRIVMSG, MSG_NOTICE, MSG_CTCP, MSG_CTCPREPLY, MSG_JOIN, MSG_PART, MSG_QUIT, MSG_MODE, MSG_TOPIC, MSG_NUMERIC, MSG_OTHER }; // TODO: finish this list

class Base {
	public:
		Base(std::string working, std::string config, std::string configFileName, unsigned short debug, bool log);
		void readConfiguration();
		void startQueueThread();
		void loadModules();
		void connectServers();
		void checkModules();
		void unloadEverything();
		// TODO: Module hooks
		// TODO: Protocol hooks
		LoadResult loadModule(std::string modName);
		void unloadModule(std::string modName);
		void connectServer(std::string server);
		void disconnectServer(std::string server);
		Socket* loadSocket(std::string sockettype);
		void messageQueue();
	private:
		const std::string workingDir, configDir, configName;
		const unsigned short debugLevel;
		const bool logDump;
		bool startup;
		std::unordered_map<std::string, std::unordered_map<std::string, std::string>> serverConfig, moduleConfig;
		std::list<std::string> startupServers, startupModules;
		std::map<std::string, Protocol*> servers;
		std::map<std::string, Module*> modules;
		std::unordered_map<std::string, void*> moduleFiles;
		std::thread queueThread;
		std::queue<std::tuple<MsgType, std::vector<std::string>, bool>> dataQueue;
};