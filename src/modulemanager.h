#pragma once
#include "main.h"
#include "config.h"
#include "ircmessage.h"
#include "mutexlocker.h"
#include "logmanager.h"
#include "socketmanager.h"
#include "modhooks.h"

class Module;
class ClientModule;
class ServerModule;
class ServerManager;

const std::set<unsigned int> modAPIVersions { 3000 };
typedef bool MsgAction;
const bool MSG_CONTINUE = true;
const bool MSG_IGNORE = false;

class ModuleManager {
	public:
		ModuleManager();
		~ModuleManager();
		void pointManagers(ServerManager* sm, SocketManager* sockm);
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
		std::function<void()> generateHookCaller(void(ModType::*func)(Args...), const std::list<std::pair<std::string, std::shared_ptr<ModType>>>& modList, Args... args);
		template<typename ModType, typename... Args>
		std::function<void()> generateMsgHookCaller(MsgAction(ModType::*func)(Args...), const std::list<std::pair<std::string, std::shared_ptr<ModType>>>& modList, Args... args);
		template<typename ModType, typename... Args>
		std::function<void()> generateChanOutHookCaller(void(ModType::*func)(Args...), const std::list<std::pair<std::string, std::shared_ptr<ModType>>>& modList, const std::string& server, const std::string& client, const std::list<std::pair<std::string, char>>& channels, const std::string& message, const std::map<std::string, std::string>& tags, const std::function<void(const std::string&, const std::list<std::pair<std::string, char>>&, const std::string&, const std::map<std::string, std::string>&)>& callback);
		template<typename ModType, typename... Args>
		std::function<void()> generateUserOutHookCaller(void(ModType::*func)(Args...), const std::list<std::pair<std::string, std::shared_ptr<ModType>>>& modList, const std::string& server, const std::string& client, const std::list<std::string>& users, const std::string& message, const std::map<std::string, std::string>& tags, const std::function<void(const std::string&, const std::list<std::string>&, const std::string&, const std::map<std::string, std::string>&)>& callback);
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
		ModuleLoadFailed(std::string&& desc) : description(std::forward<std::string>(desc)) {}
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


#include "servermanager.h"
#include "modtypes/basemodule.h"
#include "modtypes/clientmodule.h"
#include "modtypes/servermodule.h"