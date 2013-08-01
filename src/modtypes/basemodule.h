#pragma once
#include "../main.h"

class ClientModule;
class ServerModule;
class ModuleManager;
class ServerManager;
class SocketManager;
class Socket;

class Module {
	public:
		Module(const std::string& name) : moduleName(name) {}
		virtual ~Module() {}
		void loadManagerPointer(ModuleManager* mm, ServerManager* sm, SocketManager* sockm) { modmanager = mm; servmanager = sm; sockmanager = sockm; }
		virtual const unsigned int apiVersion() const = 0;
		virtual const std::string functionid() const = 0;
		virtual const bool keepAlive() const { return false; }
		virtual std::shared_ptr<ClientModule> clientModule() { return std::shared_ptr<ClientModule> (nullptr); }
		virtual std::shared_ptr<ServerModule> serverModule() { return std::shared_ptr<ServerModule> (nullptr); }
		virtual std::unordered_map<ActionType, std::unordered_map<Priority, std::set<std::string>, std::hash<int>>, std::hash<int>> registerActions() { return std::unordered_map<ActionType, std::unordered_map<Priority, std::set<std::string>, std::hash<int>>, std::hash<int>> (); }
		
		virtual const std::string description() const { return "No description provided for this module."; }
		virtual const std::list<std::string> requires() const { return std::list<std::string> (); }
		virtual const std::list<std::string> uses() const { return std::list<std::string> (); }
		virtual const std::list<std::string> provides() const { return std::list<std::string> (); }
		
		virtual void onLoadComplete() {}
		virtual void onUnload() {}
		virtual void onRehash() {}
		virtual void onModuleLoad(const std::string& module) {}
		virtual void onModuleUnload(const std::string& module) {}
	protected:
		const std::string moduleName;
		
		void connectServer(const std::string& server) { servmanager->connectServer(server); }
		void disconnectServer(const std::string& server) { servmanager->disconnectServer(server); }
		void loadModule(const std::string& modName) { modmanager->loadModule(modName); }
		void unloadModule(const std::string& modName) { modmanager->unloadModule(modName); }
		
		std::shared_ptr<Socket> assignSocket(const std::string& socketType) { return sockmanager->getSocket(socketType); }
		
		std::list<std::string> loadedModules() { return modmanager->modules(); }
		
		std::list<std::string> providedServices() { return modmanager->providedServices(); }
		bool serviceIsProvided(const std::string& service) { return modmanager->serviceIsProvided(service); }
		std::list<std::string> serviceProviders(const std::string& service) { return modmanager->serviceProviders(service); }
		std::list<std::string> serviceUsers(const std::string& service) { return modmanager->serviceUsers(service); }
		void addService(const std::string& modName, const std::string& service) { modmanager->addService(modName, service); }
		void removeService(const std::string& modName, const std::string& service) { modmanager->removeService(modName, service); }
		void addServiceClient(const std::string& modName, const std::string& service) { modmanager->addServiceClient(modName, service); }
		void removeServiceClient(const std::string& modName, const std::string& service) { modmanager->removeServiceClient(modName, service); }
		void addServiceDependency(const std::string& modName, const std::string& service) { modmanager->addServiceDependency(modName, service); }
		void removeServiceDependency(const std::string& modName, const std::string& service) { modmanager->removeServiceDependency(modName, service); }
	private:
		ModuleManager* modmanager;
		ServerManager* servmanager;
		SocketManager* sockmanager;
};