#pragma once
#include "main.h"
#include "config.h"
#include "ircmessage.h"
#include "mutexlocker.h"
#include "servermanager.h"

class Module;
class ClientModule;
class ServerModule;

const std::list<unsigned int> apiVersions { 3000 };
typedef bool MsgAction;
const bool MSG_CONTINUE = true;
const bool MSG_IGNORE = false;

enum ModeType { MODE_LIST, MODE_STATUS, MODE_PARAM_UNSET, MODE_PARAM, MODE_NOPARAM };

const int ServerHookLevel = 100;

enum Priority { PRIORITY_FIRST, PRIORITY_NORMAL, PRIORITY_BEFORE, PRIORITY_AFTER, PRIORITY_LAST };
enum ActionType {
	HOOK_CLIENT_MESSAGE_CHANNEL = 0,
	HOOK_CLIENT_MESSAGE_USER,
	HOOK_CLIENT_NOTICE_CHANNEL,
	HOOK_CLIENT_NOTICE_USER,
	HOOK_CLIENT_MODE_CHANNEL,
	HOOK_CLIENT_MODE_USER,
	HOOK_CLIENT_JOIN,
	HOOK_CLIENT_PART,
	HOOK_CLIENT_QUIT,
	HOOK_CLIENT_KICK,
	HOOK_CLIENT_TOPIC,
	HOOK_CLIENT_INVITE,
	HOOK_CLIENT_KNOCK,
	HOOK_CLIENT_NICK,
	HOOK_CLIENT_PING,
	HOOK_CLIENT_PONG,
	HOOK_CLIENT_AWAY,
	HOOK_CLIENT_UNAWAY,
	HOOK_CLIENT_NUMERIC,
	HOOK_CLIENT_WALLOPS,
	HOOK_CLIENT_CAP,
	HOOK_CLIENT_CONNECT_SELF,
	HOOK_CLIENT_QUIT_SELF,
	HOOK_CLIENT_OTHER,
	HOOK_CLIENT_OUT_MESSAGE_CHANNEL,
	HOOK_CLIENT_OUT_MESSAGE_USER,
	HOOK_CLIENT_OUT_NOTICE_CHANNEL,
	HOOK_CLIENT_OUT_NOTICE_USER,
	HOOK_CLIENT_SEND_MESSAGE_CHANNEL,
	HOOK_CLIENT_SEND_MESSAGE_USER,
	HOOK_CLIENT_SEND_NOTICE_CHANNEL,
	HOOK_CLIENT_SEND_NOTICE_USER,
	
	HOOK_SERVER_MESSAGE_CHANNEL = ServerHookLevel,
	HOOK_SERVER_MESSAGE_USER,
	HOOK_SERVER_NOTICE_CHANNEL,
	HOOK_SERVER_NOTICE_USER,
	HOOK_SERVER_MODE_CHANNEL,
	HOOK_SERVER_MODE_USER,
	HOOK_SERVER_JOIN,
	HOOK_SERVER_PART,
	HOOK_SERVER_KICK,
	HOOK_SERVER_TOPIC,
	HOOK_SERVER_INVITE,
	HOOK_SERVER_KNOCK,
	HOOK_SERVER_NICK,
	HOOK_SERVER_PING,
	HOOK_SERVER_PONG,
	HOOK_SERVER_AWAY,
	HOOK_SERVER_UNAWAY,
	HOOK_SERVER_NOTICE_SERVER,
	HOOK_SERVER_METADATA_CHANNEL,
	HOOK_SERVER_METADATA_USER,
	HOOK_SERVER_XLINE_ADD,
	HOOK_SERVER_XLINE_REMOVE,
	HOOK_SERVER_CONNECT_USER,
	HOOK_SERVER_QUIT_USER,
	HOOK_SERVER_CONNECT_USER_SELF,
	HOOK_SERVER_QUIT_USER_SELF,
	HOOK_SERVER_CHANGE_IDENT,
	HOOK_SERVER_CHANGE_HOST,
	HOOK_SERVER_CHANGE_GECOS,
	HOOK_SERVER_WALLOPS,
	HOOK_SERVER_CONNECT_SERVER,
	HOOK_SERVER_QUIT_SERVER,
	HOOK_SERVER_CAPAB,
	HOOK_SERVER_BURST,
	HOOK_SERVER_ENDBURST,
	HOOK_SERVER_CONNECT_SELF,
	HOOK_SERVER_QUIT_SELF,
	HOOK_SERVER_OTHER,
	HOOK_SERVER_OUT_MESSAGE_CHANNEL,
	HOOK_SERVER_OUT_MESSAGE_USER,
	HOOK_SERVER_OUT_NOTICE_CHANNEL,
	HOOK_SERVER_OUT_NOTICE_USER,
	HOOK_SERVER_SEND_MESSAGE_CHANNEL,
	HOOK_SERVER_SEND_MESSAGE_USER,
	HOOK_SERVER_SEND_NOTICE_CHANNEL,
	HOOK_SERVER_SEND_NOTICE_USER
};

class ModuleManager {
	public:
		ModuleManager();
		void pointServerManager(ServerManager* sm);
		void loadStartupModules();
		void loadModule(const std::string& name);
		void unloadModule(const std::string& name);
		bool checkKeepAlive() const;
		
		std::shared_ptr<Module> modulePointer(const std::string& modName) const;
		
		void addService(const std::string& modName, const std::string& service);
		void removeService(const std::string& modName, const std::string& service);
		void addServiceClient(const std::string& modName, const std::string& service);
		void removeServiceClient(const std::string& modName, const std::string& service);
		void addServiceDependency(const std::string& modName, const std::string& service);
		void removeServiceDependency(const std::string& modName, const std::string& service);
		
		std::list<std::string> providingModules(const std::string& capability) const;
		std::list<std::string> usingModules(const std::string& capability) const;
		
		template<ActionType, const std::string&, typename... Args> void callHook(ActionType type, const Args&... args);
		
		void rehash();
		
		std::shared_ptr<Socket> assignSocket(const std::string& socketType);
		
		std::list<std::string> modules();
		
		std::list<std::string> providedServices();
		bool serviceIsProvided(const std::string& service);
		std::list<std::string> serviceProviders(const std::string& service);
		std::list<std::string> serviceUsers(const std::string& service);
	private:
		std::shared_ptr<Module> openModule(const std::string& name);
		void verifyModule(std::shared_ptr<Module> mod);
		std::unordered_map<std::string, std::shared_ptr<Module>> loadedModules;
		std::unordered_map<std::string, std::unordered_map<ActionType, std::unordered_map<Priority, std::list<std::string>, std::hash<int>>, std::hash<int>>> actionPriority;
		std::unordered_map<std::string, std::list<std::string>> providers;
		std::unordered_map<std::string, std::list<std::string>> clients;
		std::unordered_map<std::string, std::list<std::string>> dependents;
		ServerManager* servers;
		
		std::mutex queueMutex;
		std::queue<std::function<void()>> actionQueue;
		
		void processQueue();
		bool runningProcess = false;
};

class ServiceNotLoaded : public std::exception {
	public:
		const char* what() const noexcept { return "The required service you are trying to add is not loaded."; }
};


#include "modtypes/basemodule.h"
#include "modtypes/clientmodule.h"
#include "modtypes/servermodule.h"