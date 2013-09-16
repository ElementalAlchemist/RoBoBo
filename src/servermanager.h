#pragma once
#include "main.h"
#include "modulemanager.h"
#include "socketmanager.h"
#include "modtypes/clientprotocol.h"
#include "modtypes/serverprotocol.h"

const std::set<unsigned int> protoAPIVersions { 3000 };

class ServerManager {
	public:
		void pointManagers(ModuleManager* mm, SocketManager* sm) { modmanager = mm; sockmanager = sm; }
		void connectStartupServers();
		void connectServer(const std::string& server);
		void disconnectServer(const std::string& server);
		size_t checkServers();
		
		void sendMsg(const std::string& server, const std::string& source, const std::string& target, const std::string& message, const std::map<std::string, std::string>& tags) { callEitherHook(&ClientProtocol::sendMsg, &ServerProtocol::sendMsg, server, std::forward<const std::string&>(source), std::forward<const std::string&>(target), std::forward<const std::string&>(message), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void sendNotice(const std::string& server, const std::string& source, const std::string& target, const std::string& message, const std::map<std::string, std::string>& tags) { callEitherHook(&ClientProtocol::sendNotice, &ServerProtocol::sendNotice, server, std::forward<const std::string&>(source), std::forward<const std::string&>(target), std::forward<const std::string&>(message), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void setMode(const std::string& server, const std::string& source, const std::string& target, const std::list<std::tuple<bool, std::string, std::string>>& modes, const std::map<std::string, std::string>& tags) { callEitherHook(&ClientProtocol::setMode, &ServerProtocol::setMode, server, std::forward<const std::string&>(source), std::forward<const std::string&>(target), std::forward<const std::list<std::tuple<bool, std::string, std::string>>&>(modes), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void joinChan(const std::string& server, const std::string& client, const std::string& channel, const std::map<std::string, std::string>& tags) { callServerHook(&ServerProtocol::joinChan, server, std::forward<const std::string&>(client), std::forward<const std::string&>(channel), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void joinChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& key, const std::map<std::string, std::string>& tags) { callClientHook(&ClientProtocol::joinChan, server, std::forward<const std::string&>(client), std::forward<const std::string&>(channel), std::forward<const std::string&>(key), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void joinChanOver(const std::string& server, const std::string& client, const std::string& channel, const std::list<std::string>& statuses, time_t timestamp, const std::map<std::string, std::string>& tags) { callServerHook(&ServerProtocol::joinChanOver, server, std::forward<const std::string&>(client), std::forward<const std::string&>(channel), std::forward<const std::list<std::string>&>(statuses), std::forward<time_t>(timestamp), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void partChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason, const std::map<std::string, std::string>& tags) { callEitherHook(&ClientProtocol::partChan, &ServerProtocol::partChan, server, std::forward<const std::string&>(client), std::forward<const std::string&>(channel), std::forward<const std::string&>(reason), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void kickUser(const std::string& server, const std::string& source, const std::string& channel, const std::string& user, const std::string& reason, const std::map<std::string, std::string>& tags) { callEitherHook(&ClientProtocol::kickUser, &ServerProtocol::kickUser, server, std::forward<const std::string&>(source), std::forward<const std::string&>(channel), std::forward<const std::string&>(user), std::forward<const std::string&>(reason), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void setTopic(const std::string& server, const std::string& source, const std::string& channel, const std::string& topic, const std::map<std::string, std::string>& tags) { callEitherHook(&ClientProtocol::setTopic, &ServerProtocol::setTopic, server, std::forward<const std::string&>(source), std::forward<const std::string&>(channel), std::forward<const std::string&>(topic), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void inviteUser(const std::string& server, const std::string& source, const std::string& channel, const std::string& user, const std::map<std::string, std::string>& tags) { callEitherHook(&ClientProtocol::inviteUser, &ServerProtocol::inviteUser, server, std::forward<const std::string&>(source), std::forward<const std::string&>(channel), std::forward<const std::string&>(user), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void knock(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason, const std::map<std::string, std::string>& tags) { callEitherHook(&ClientProtocol::knock, &ServerProtocol::knock, server, std::forward<const std::string&>(client), std::forward<const std::string&>(channel), std::forward<const std::string&>(reason), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void changeNick(const std::string& server, const std::string& client, const std::string& nick, const std::map<std::string, std::string>& tags) { callEitherHook(&ClientProtocol::changeNick, &ServerProtocol::changeNick, server, std::forward<const std::string&>(client), std::forward<const std::string&>(nick), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void sendPing(const std::string& server, const std::string& destServer, const std::map<std::string, std::string>& tags) { callServerHook(&ServerProtocol::sendPing, server, std::forward<const std::string&>(destServer), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void sendPing(const std::string& server, const std::string& client, const std::string& data, const std::map<std::string, std::string>& tags) { callClientHook(&ClientProtocol::sendPing, server, std::forward<const std::string&>(client), std::forward<const std::string&>(data), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void setAway(const std::string& server, const std::string& client, const std::string& reason, const std::map<std::string, std::string>& tags) { callEitherHook(&ClientProtocol::setAway, &ServerProtocol::setAway, server, std::forward<const std::string&>(client), std::forward<const std::string&>(reason), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void setUnaway(const std::string& server, const std::string& client, const std::map<std::string, std::string>& tags) { callEitherHook(&ClientProtocol::setUnaway, &ServerProtocol::setUnaway, server, std::forward<const std::string&>(client), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void oper(const std::string& server, const std::string& client, const std::string& username, const std::string& password, const std::map<std::string, std::string>& tags) { callClientHook(&ClientProtocol::oper, server, std::forward<const std::string&>(client), std::forward<const std::string&>(username), std::forward<const std::string&>(password), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void oper(const std::string& server, const std::string& client, const std::string& type, const std::map<std::string, std::string>& tags) { callServerHook(&ServerProtocol::oper, server, std::forward<const std::string&>(client), std::forward<const std::string&>(type), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void sendServerNotice(const std::string& server, const std::string& type, const std::string& message, const std::map<std::string, std::string>& tags) { callServerHook(&ServerProtocol::sendServerNotice, server, std::forward<const std::string&>(type), std::forward<const std::string&>(message), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void setMetadata(const std::string& server, const std::string& target, const std::string& key, const std::string& value, const std::map<std::string, std::string>& tags) { callServerHook(&ServerProtocol::setMetadata, server, std::forward<const std::string&>(target), std::forward<const std::string&>(key), std::forward<const std::string&>(value), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void setXLine(const std::string& server, const std::string& source, const std::string& lineType, const std::string& mask, time_t duration, const std::string& reason, const std::map<std::string, std::string>& tags) { callServerHook(&ServerProtocol::setXLine, server, std::forward<const std::string&>(source), std::forward<const std::string&>(lineType), std::forward<const std::string&>(mask), std::forward<time_t>(duration), std::forward<const std::string&>(reason), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void removeXLine(const std::string& server, const std::string& source, const std::string& lineType, const std::string& mask, const std::map<std::string, std::string>& tags) { callServerHook(&ServerProtocol::removeXLine, server, std::forward<const std::string&>(source), std::forward<const std::string&>(lineType), std::forward<const std::string&>(mask), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void changeIdent(const std::string& server, const std::string& user, const std::string& ident, const std::map<std::string, std::string>& tags) { callServerHook(&ServerProtocol::changeIdent, server, std::forward<const std::string&>(user), std::forward<const std::string&>(ident), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void changeHost(const std::string& server, const std::string& user, const std::string& host, const std::map<std::string, std::string>& tags) { callServerHook(&ServerProtocol::changeHost, server, std::forward<const std::string&>(user), std::forward<const std::string&>(host), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void changeGecos(const std::string& server, const std::string& user, const std::string& gecos, const std::map<std::string, std::string>& tags) { callServerHook(&ServerProtocol::changeGecos, server, std::forward<const std::string&>(user), std::forward<const std::string&>(gecos), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void sendWallops(const std::string& server, const std::string& source, const std::string& message, const std::map<std::string, std::string>& tags) { callEitherHook(&ClientProtocol::sendWallops, &ServerProtocol::sendWallops, server, std::forward<const std::string&>(source), std::forward<const std::string&>(message), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void pushLine(const std::string& server, const std::string& user, const IRCMessage* line) { callServerHook(&ServerProtocol::pushLine, server, std::forward<const std::string&>(user), std::forward<const IRCMessage*>(line)); }
		void sendOtherData(const std::string& server, const std::string& client, const IRCMessage* line) { callClientHook(&ClientProtocol::sendOtherData, server, std::forward<const std::string&>(client), std::forward<const IRCMessage*>(line)); }
		void sendOtherData(const std::string& server, const IRCMessage* line) { callServerHook(&ServerProtocol::sendOtherData, server, std::forward<const IRCMessage*>(line)); }
		
		std::string addClient(const std::string& server, const std::string& nick, const std::string& ident, const std::string& host, const std::string& gecos, const std::map<std::string, std::string>& tags) { return callServerHook(&ServerProtocol::addClient, server, std::forward<const std::string&>(nick), std::forward<const std::string&>(ident), std::forward<const std::string&>(host), std::forward<const std::string&>(gecos), std::forward<const std::map<std::string, std::string>&>(tags)); }
		std::string addClient(const std::string& server, const std::string& nick, const std::string& ident, const std::string& gecos) { return callClientHook(&ClientProtocol::addClient, server, std::forward<const std::string&>(nick), std::forward<const std::string&>(ident), std::forward<const std::string&>(gecos)); }
		void removeClient(const std::string& server, const std::string& client, const std::map<std::string, std::string>& tags) { callServerHook(&ServerProtocol::removeClient, server, std::forward<const std::string&>(client), std::forward<const std::map<std::string, std::string>&>(tags)); }
		void removeClient(const std::string& server, const std::string& client) { callClientHook(&ClientProtocol::removeClient, server, std::forward<const std::string&>(client)); }
		
		std::list<std::pair<std::string, bool>> activeServers(bool requestedByServerModule);
		std::list<std::pair<std::string, bool>> connectedServers(bool requestedByServerModule);
		std::string serverType(const std::string& server);
		
		std::list<std::string> networkServerList(const std::string& server) { return callServerHook(&ServerProtocol::networkServerList, server); }
		std::list<std::string> xLineTypes(const std::string& server) { return callServerHook(&ServerProtocol::xLineTypes, server); }
		std::list<std::string> xLineList(const std::string& server, const std::string& lineType) { return callServerHook(&ServerProtocol::xLineList, server, std::forward<const std::string&>(lineType)); }
		time_t xLineExpiry(const std::string& server, const std::string& lineType, const std::string& mask) { return callServerHook(&ServerProtocol::xLineExpiry, server, std::forward<const std::string&>(lineType), std::forward<const std::string&>(mask)); }
		std::string xLineReason(const std::string& server, const std::string& lineType, const std::string& mask) { return callServerHook(&ServerProtocol::xLineReason, server, std::forward<const std::string&>(lineType), std::forward<const std::string&>(mask)); }
		
		std::set<std::string> chanTypes(const std::string& server) { return callEitherHook(&ClientProtocol::chanTypes, &ServerProtocol::chanTypes, server); }
		std::list<std::pair<ModeType, std::string>> allChanModes(const std::string& server) { return callEitherHook(&ClientProtocol::allChanModes, &ServerProtocol::allChanModes, server); }
		char prefixSymbol(const std::string& server, const std::string& mode) { return callEitherHook(&ClientProtocol::prefixSymbol, &ServerProtocol::prefixSymbol, server, std::forward<const std::string&>(mode)); }
		std::pair<std::string, char> compareStatus(const std::string& server, const std::string& status0, const std::string& status1) { std::pair<std::string, char>(ClientProtocol::*clientFunc)(const std::string&, const std::string&) = &ClientProtocol::compareStatus; std::pair<std::string, char>(ServerProtocol::*serverFunc)(const std::string&, const std::string&) = &ServerProtocol::compareStatus; return callEitherHook(clientFunc, serverFunc, server, std::forward<const std::string&>(status0), std::forward<const std::string&>(status1)); }
		std::pair<std::string, char> compareStatus(const std::string& server, const std::string& status0, char status1) { std::pair<std::string, char>(ClientProtocol::*clientFunc)(const std::string&, char) = &ClientProtocol::compareStatus; std::pair<std::string, char>(ServerProtocol::*serverFunc)(const std::string&, char) = &ServerProtocol::compareStatus; return callEitherHook(clientFunc, serverFunc, server, std::forward<const std::string&>(status0), std::forward<char>(status1)); }
		std::pair<std::string, char> compareStatus(const std::string& server, char status0, char status1) { std::pair<std::string, char>(ClientProtocol::*clientFunc)(char, char) = &ClientProtocol::compareStatus; std::pair<std::string, char>(ServerProtocol::*serverFunc)(char, char) = &ServerProtocol::compareStatus; return callEitherHook(clientFunc, serverFunc, server, std::forward<char>(status0), std::forward<char>(status1)); }
		
		std::list<std::string> chanList(const std::string& server) { return callServerHook(&ServerProtocol::chanList, server); }
		std::string chanTopic(const std::string& server, const std::string& channel) { return callEitherHook(&ClientProtocol::chanTopic, &ServerProtocol::chanTopic, server, std::forward<const std::string&>(channel)); }
		std::string chanTopicSetter(const std::string& server, const std::string& channel) { return callEitherHook(&ClientProtocol::chanTopicSetter, &ServerProtocol::chanTopicSetter, server, std::forward<const std::string&>(channel)); }
		time_t chanTimestamp(const std::string& server, const std::string& channel) { return callEitherHook(&ClientProtocol::chanTimestamp, &ServerProtocol::chanTimestamp, server, std::forward<const std::string&>(channel)); }
		std::list<std::string> chanUsers(const std::string& server, const std::string& channel) { return callEitherHook(&ClientProtocol::chanUsers, &ServerProtocol::chanUsers, server, std::forward<const std::string&>(channel)); }
		bool userInChan(const std::string& server, const std::string& channel, const std::string& user) { return callEitherHook(&ClientProtocol::userInChan, &ServerProtocol::userInChan, server, std::forward<const std::string&>(channel), std::forward<const std::string&>(user)); }
		std::pair<std::string, char> userStatus(const std::string& server, const std::string& channel, const std::string& user) { return callEitherHook(&ClientProtocol::userStatus, &ServerProtocol::userStatus, server, std::forward<const std::string&>(channel), std::forward<const std::string&>(user)); }
		bool userHasStatus(const std::string& server, const std::string& channel, const std::string& user, const std::string& status) { bool(ClientProtocol::*clientFunc)(const std::string&, const std::string&, const std::string&) = &ClientProtocol::userHasStatus; bool(ServerProtocol::*serverFunc)(const std::string&, const std::string&, const std::string&) = &ServerProtocol::userHasStatus; return callEitherHook(clientFunc, serverFunc, server, std::forward<const std::string&>(channel), std::forward<const std::string&>(user), std::forward<const std::string&>(status)); }
		bool userHasStatus(const std::string& server, const std::string& channel, const std::string& user, char status) { bool(ClientProtocol::*clientFunc)(const std::string&, const std::string&, char) = &ClientProtocol::userHasStatus; bool(ServerProtocol::*serverFunc)(const std::string&, const std::string&, char) = &ServerProtocol::userHasStatus; return callEitherHook(clientFunc, serverFunc, server, std::forward<const std::string&>(channel), std::forward<const std::string&>(user), std::forward<char>(status)); }
		bool userHasStatusOrGreater(const std::string& server, const std::string& channel, const std::string& user, const std::string& status) { bool(ClientProtocol::*clientFunc)(const std::string&, const std::string&, const std::string&) = &ClientProtocol::userHasStatusOrGreater; bool(ServerProtocol::*serverFunc)(const std::string&, const std::string&, const std::string&) = &ServerProtocol::userHasStatusOrGreater; return callEitherHook(clientFunc, serverFunc, server, std::forward<const std::string&>(channel), std::forward<const std::string&>(user), std::forward<const std::string&>(status)); }
		bool userHasStatusOrGreater(const std::string& server, const std::string& channel, const std::string& user, char status) { bool(ClientProtocol::*clientFunc)(const std::string&, const std::string&, char) = &ClientProtocol::userHasStatusOrGreater; bool(ServerProtocol::*serverFunc)(const std::string&, const std::string&, char) = &ServerProtocol::userHasStatusOrGreater; return callEitherHook(clientFunc, serverFunc, server, std::forward<const std::string&>(channel), std::forward<const std::string&>(user), std::forward<char>(status)); }
		std::map<std::string, std::string> chanModes(const std::string& server, const std::string& channel) { return callEitherHook(&ClientProtocol::chanModes, &ServerProtocol::chanModes, server, std::forward<const std::string&>(channel)); }
		bool chanHasMode(const std::string& server, const std::string& channel, const std::string& mode) { return callEitherHook(&ClientProtocol::chanHasMode, &ServerProtocol::chanHasMode, server, std::forward<const std::string&>(channel), std::forward<const std::string&>(mode)); }
		std::string chanModeParam(const std::string& server, const std::string& channel, const std::string& mode) { return callEitherHook(&ClientProtocol::chanModeParam, &ServerProtocol::chanModeParam, server, std::forward<const std::string&>(channel), std::forward<const std::string&>(mode)); }
		std::list<std::string> chanListModeList(const std::string& server, const std::string& channel, const std::string& listMode) { return callServerHook(&ServerProtocol::chanListModeList, server, std::forward<const std::string&>(channel), std::forward<const std::string&>(listMode)); }
		
		std::list<std::string> clientList(const std::string& server) { return callEitherHook(&ClientProtocol::clientList, &ServerProtocol::clientList, server); }
		std::string userNick(const std::string& server, const std::string& user) { return callEitherHook(&ClientProtocol::userNick, &ServerProtocol::userNick, server, std::forward<const std::string&>(user)); }
		std::string userIdent(const std::string& server, const std::string& user) { return callEitherHook(&ClientProtocol::userIdent, &ServerProtocol::userIdent, server, std::forward<const std::string&>(user)); }
		std::string userHost(const std::string& server, const std::string& user) { return callEitherHook(&ClientProtocol::userHost, &ServerProtocol::userHost, server, std::forward<const std::string&>(user)); }
		std::string userGecos(const std::string& server, const std::string& user) { return callEitherHook(&ClientProtocol::userGecos, &ServerProtocol::userGecos, server, std::forward<const std::string&>(user)); }
		std::map<std::string, std::string> userModes(const std::string& server, const std::string& user) { return callEitherHook(&ClientProtocol::userModes, &ServerProtocol::userModes, server, std::forward<const std::string&>(user)); }
		bool userHasMode(const std::string& server, const std::string& user, const std::string& mode) { return callEitherHook(&ClientProtocol::userHasMode, &ServerProtocol::userHasMode, server, std::forward<const std::string&>(user), std::forward<const std::string&>(mode)); }
		std::set<std::string> userChans(const std::string& server, const std::string& user) { return callEitherHook(&ClientProtocol::userChans, &ServerProtocol::userChans, server, std::forward<const std::string&>(user)); }
		time_t userTimestamp(const std::string& server, const std::string& user) { return callServerHook(&ServerProtocol::userTimestamp, server, std::forward<const std::string&>(user)); }
		time_t userNickTimestamp(const std::string& server, const std::string& user) { return callServerHook(&ServerProtocol::userNickTimestamp, server, std::forward<const std::string&>(user)); }
		time_t userAwayTimestamp(const std::string& server, const std::string& user) { return callServerHook(&ServerProtocol::userAwayTimestamp, server, std::forward<const std::string&>(user)); }
		
		std::shared_ptr<Socket> assignSocket(const std::string& socketType) { return sockmanager->getSocket(socketType); }
	private:
		std::unordered_map<std::string, std::shared_ptr<ClientProtocol>> clientServers;
		std::unordered_map<std::string, std::shared_ptr<ServerProtocol>> serverServers;
		std::unordered_map<std::string, std::string> serverTypes;
		ModuleManager* modmanager;
		SocketManager* sockmanager;
		
		template<typename Protocol>
		void unloadServer(const std::string& type, void* protoFile, Protocol* serverPtr);
		template<typename RetVal, typename... Args>
		RetVal callEitherHook(RetVal(ClientProtocol::*clientFunc)(Args...), RetVal(ServerProtocol::*serverFunc)(Args...), const std::string& server, Args&&... args);
		template<typename RetVal, typename... Args>
		RetVal callClientHook(RetVal(ClientProtocol::*func)(Args...), const std::string& server, Args&&... args);
		template<typename RetVal, typename... Args>
		RetVal callServerHook(RetVal(ServerProtocol::*func)(Args...), const std::string& server, Args&&... args);
};

class ServerNotConfigured : public std::exception {
	public:
		const char* what() const noexcept { return "The server is not defined in the configuration file."; }
};

class ServerNoProtocol : public std::exception {
	public:
		const char* what() const noexcept { return "There is no protocol defined for the server."; }
};

class ProtoLoadFailed : public std::exception {
	public:
		ProtoLoadFailed(std::string&& desc) : description(std::forward<const std::string>(desc)) {}
		const char* what() const noexcept { return description.c_str(); }
	private:
		const std::string description;
};

class ProtoAPIMismatch : public std::exception {
	public:
		const char* what() const noexcept { return "The protocol module is not compatible with the current module API."; }
};

class ServerNotLoaded : public std::exception {
	public:
		const char* what() const noexcept { return "The selected server is not loaded."; }
};

class ServerDoesNotSupport : public std::exception {
	public:
		const char* what() const noexcept { return "The server does not support the request."; }
};