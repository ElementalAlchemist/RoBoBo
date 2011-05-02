#ifndef BASE_H
#define BASE_H
#include "modules.h"

typedef void* (*module_spawn_t)(std::tr1::unordered_map<std::string, std::string>, Base*, std::string, unsigned short);
typedef void* (*proto_spawn_t)(std::string, std::tr1::unordered_map<std::string, std::string>, Base*, unsigned short);
typedef void* (*socket_spawn_t)();

class Base {
	public:
		Base(std::string confdir, std::string confname, unsigned short debug);
		void sendToServer(std::string server, std::string rawLine);
		std::tr1::unordered_map<std::string, std::string> serverData(std::string server);
		std::vector<std::vector<char> > serverChanModes(std::string server);
		std::list<std::pair<char, char> > serverPrefixes(std::string server);
		void callPreHook(std::string server, std::vector<std::string> parsedLine);
		void callPostHook(std::string server, std::vector<std::string> parsedLine);
		std::string callHookOut(std::string server, std::vector<std::string> parsedLine);
		void callHookSend(std::string server, std::vector<std::string> parsedLine);
		void callPreConnectHook(std::string server);
		void callConnectHook(std::string server);
		void callQuitHook(std::string server);
		void callPreModulesHook(std::string server, std::vector<std::string> parsedLine, std::tr1::unordered_map<std::string, Module*>* modules);
		void callPostModulesHook(std::string server, std::vector<std::string> parsedLine, std::tr1::unordered_map<std::string, Module*>* modules);
		std::string callModulesHookOut(std::string server, std::vector<std::string> parsedLine, std::tr1::unordered_map<std::string, Module*>* modules);
		void callModulesHookSend(std::string server, std::vector<std::string> parsedLine, std::tr1::unordered_map<std::string, Module*>* modules);
		void callPreConnectModulesHook(std::string server, std::tr1::unordered_map<std::string, Module*>* modules);
		void callConnectModulesHook(std::string server, std::tr1::unordered_map<std::string, Module*>* modules);
		void callQuitModulesHook(std::string server, std::tr1::unordered_map<std::string, Module*>* modules);
		bool isChanType(char chanPrefix, std::string server);
		std::tr1::unordered_map<std::string, Module*> loadedModules();
		std::list<std::string> serverList();
		std::multimap<std::string, std::string> moduleAbilities();
		std::tr1::unordered_map<std::string, std::vector<std::string> > moduleSupports();
		bool serverIsClient(std::string server);
		std::list<std::string> channels(std::string server);
		std::string channelTopic(std::string server, std::string channel);
		std::list<std::string> channelUsers(std::string server, std::string channel);
		std::string userIdent(std::string server, std::string user);
		std::string userHost(std::string server, std::string user);
		std::pair<char, char> userStatus(std::string server, std::string channel, std::string user);
		Socket* assignSocket(std::string socketType);
		void rehash();
		bool connectServer(std::string serverName);
		bool loadModule(std::string modName, bool startup);
		void unloadModule(std::string modName);
		static void* serverCheck_thread(void* ptr);
		void serverCheck();
	private:
		std::tr1::unordered_map<std::string, Protocol*> servers;
		std::tr1::unordered_map<std::string, Module*> highModules, mediumHighModules, normalModules, mediumLowModules, lowModules;
		std::tr1::unordered_map<std::string, void*> moduleFiles;
		std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > serverConfigs, moduleConfigs;
		std::multimap<std::string, std::string> modAbilities;
		std::tr1::unordered_map<std::string, std::vector<std::string> > modSupports;
		std::tr1::unordered_map<std::string, void*> protocolFiles;
		std::tr1::unordered_map<std::string, void*> socketFiles;
		unsigned short debugLevel;
		std::string directory;
		std::string configName;
		bool charIsNumeric(char number);
		static void* tUnloadMod_thread(void* mip);
		void tUnloadMod();
		std::vector<std::string> moduleToUnload;
		pthread_t serverCheckThread;
		pthread_attr_t detachedState;
};
#endif