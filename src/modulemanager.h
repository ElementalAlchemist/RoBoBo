#pragma once
#include "main.h"
#include "config.h"
#include "ircmessage.h"
#include "mutexlocker.h"
#include "logmanager.h"
#include "servermanager.h"
#include "modtypes/basemodule.h"
#include "modtypes/clientmodule.h"
#include "modtypes/servermodule.h"

const std::set<unsigned int> apiVersions { 3000 };
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
	HOOK_SERVER_CREATE_CHANNEL,
	HOOK_SERVER_DESTROY_CHANNEL,
	HOOK_SERVER_REQUEST_STATS,
	HOOK_SERVER_REQUEST_MOTD,
	HOOK_SERVER_REQUEST_TIME,
	HOOK_SERVER_REQUEST_ADMIN,
	HOOK_SERVER_REQUEST_INFO,
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
		void pointSocketManager(SocketManager* sm);
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
		
		template<typename... Args>
		void callClientHook(ActionType type, Args... args);
		template<typename... Args>
		void callServerHook(ActionType type, Args... args);
		
		void onRehash();
		
		std::shared_ptr<Socket> assignSocket(const std::string& socketType);
		
		std::list<std::string> modules();
		
		std::list<std::string> providedServices();
		bool serviceIsProvided(const std::string& service);
		std::list<std::string> serviceProviders(const std::string& service);
		std::list<std::string> serviceUsers(const std::string& service);
	private:
		std::shared_ptr<Module> openModule(const std::string& name);
		void verifyModule(const std::string& name, std::shared_ptr<Module> mod);
		std::unordered_map<std::string, std::shared_ptr<Module>> loadedModules;
		std::unordered_map<std::string, std::shared_ptr<ClientModule>> clientModules;
		std::unordered_map<std::string, std::shared_ptr<ServerModule>> serverModules;
		std::unordered_map<ActionType, std::list<std::string>, std::hash<int>> registeredActions;
		std::unordered_map<std::string, std::unordered_map<ActionType, std::unordered_map<Priority, std::set<std::string>, std::hash<int>>, std::hash<int>>> actionPriority;
		std::unordered_map<std::string, std::list<std::string>> providers;
		std::unordered_map<std::string, std::list<std::string>> clients;
		std::unordered_map<std::string, std::list<std::string>> dependents;
		std::unordered_map<std::string, void*> moduleFiles;
		ServerManager* servers;
		SocketManager* sockets;
		
		std::mutex queueMutex;
		std::queue<std::function<void()>> actionQueue;
		
		template<typename ModType, typename... Args>
		std::function<void()> generateHookCaller(void(ModType::*func)(Args...), const std::list<std::shared_ptr<ModType>>& modList, Args... args);
		template<typename ModType, typename... Args>
		std::function<void()> generateMsgHookCaller(MsgAction(ModType::*func)(Args...), const std::list<std::shared_ptr<ModType>>& modList, Args... args);
		template<typename ModType, typename... Args>
		std::function<void()> generateChanOutHookCaller(void(ModType::*func)(Args...), const std::list<std::shared_ptr<ModType>>& modList, Args... args);
		template<typename ModType, typename... Args>
		std::function<void()> generateUserOutHookCaller(void(ModType::*func)(Args...), const std::list<std::shared_ptr<ModType>>& modList, Args... args);
		void startQueue();
		void processQueue();
		bool runningProcess = false;
};

class ServiceNotLoaded : public std::exception {
	public:
		const char* what() const noexcept { return "The required service you are trying to add is not loaded."; }
};

class HookTypeException : public std::exception {
	public:
		const char* what() const noexcept { return "A client or server type of hook was given, but the other type was requested."; }
};

class ModuleAlreadyLoaded : public std::exception {
	public:
		const char* what() const noexcept { return "The module is already loaded."; }
};

class ModuleLoadFailed : public std::exception {
	public:
		ModuleLoadFailed(const std::string& desc) : description(desc) {}
		const char* what() const noexcept { return description.c_str(); }
	private:
		const std::string description;
};

class ModuleAPIMismatch : public std::exception {
	public:
		const char* what() const noexcept { return "The module is not compatible with the current module API."; }
};

class ModuleRequirementsNotMet : public std::exception {
	public:
		const char* what() const noexcept { return "The module's service requirements were not fulfilled by loaded modules."; }
};

class ModuleNotLoaded : public std::exception {
	public:
		const char* what() const noexcept { return "The module being unloaded wasn't already loaded."; }
};