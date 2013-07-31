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
		
		void sendMsg(const std::string& server, const std::string& source, const std::string& target, const std::string& message, const std::map<std::string, std::string>& tags) { callEitherHook(&ClientProtocol::sendMsg, &ServerProtocol::sendMsg, server, source, target, message, tags); }
		void sendNotice(const std::string& server, const std::string& source, const std::string& target, const std::string& message, const std::map<std::string, std::string>& tags) { callEitherHook(&ClientProtocol::sendNotice, &ServerProtocol::sendNotice, server, source, target, message, tags); }
		void setMode(const std::string& server, const std::string& source, const std::string& target, const std::list<std::tuple<bool, std::string, std::string>>& modes, const std::map<std::string, std::string>& tags) { callEitherHook(&ClientProtocol::setMode, &ServerProtocol::setMode, server, source, target, modes, tags); }
		void joinChan(const std::string& server, const std::string& client, const std::string& channel, const std::map<std::string, std::string>& tags) { callServerHook(&ServerProtocol::joinChan, server, client, channel, tags); }
		void joinChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& key, const std::map<std::string, std::string>& tags) { callClientHook(&ClientProtocol::joinChan, server, client, channel, key, tags); }
		void joinChanOver(const std::string& server, const std::string& client, const std::string& channel, const std::list<std::string>& statuses, time_t timestamp, const std::map<std::string, std::string>& tags) { callServerHook(&ServerProtocol::joinChanOver, server, client, channel, statuses, timestamp, tags); }
		void partChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason, const std::map<std::string, std::string>& tags) { callEitherHook(&ClientProtocol::partChan, &ServerProtocol::partChan, server, client, channel, reason, tags); }
		void kickUser(const std::string& server, const std::string& source, const std::string& channel, const std::string& user, const std::string& reason, const std::map<std::string, std::string>& tags) { callEitherHook(&ClientProtocol::kickUser, &ServerProtocol::kickUser, server, source, channel, user, reason, tags); }
		void setTopic(const std::string& server, const std::string& source, const std::string& channel, const std::string& topic, const std::map<std::string, std::string>& tags) { callEitherHook(&ClientProtocol::setTopic, &ServerProtocol::setTopic, server, source, channel, topic, tags); }
		void inviteUser(const std::string& server, const std::string& source, const std::string& channel, const std::string& user, const std::map<std::string, std::string>& tags) { callEitherHook(&ClientProtocol::inviteUser, &ServerProtocol::inviteUser, server, source, channel, user, tags); }
		void knock(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason, const std::map<std::string, std::string>& tags) { callEitherHook(&ClientProtocol::knock, &ServerProtocol::knock, server, client, channel, reason, tags); }
		void changeNick(const std::string& server, const std::string& client, const std::string& nick, const std::map<std::string, std::string>& tags) { callEitherHook(&ClientProtocol::changeNick, &ServerProtocol::changeNick, server, client, nick, tags); }
		void sendPing(const std::string& server, const std::string& data, const std::map<std::string, std::string>& tags) { callEitherHook(&ClientProtocol::sendPing, &ServerProtocol::sendPing, server, data, tags); }
		void setAway(const std::string& server, const std::string& client, const std::string& reason, const std::map<std::string, std::string>& tags) { callEitherHook(&ClientProtocol::setAway, &ServerProtocol::setAway, server, client, reason, tags); }
		void setUnaway(const std::string& server, const std::string& client, const std::map<std::string, std::string>& tags) { callEitherHook(&ClientProtocol::setUnaway, &ServerProtocol::setUnaway, server, client, tags); }
		void oper(const std::string& server, const std::string& client, const std::string& username, const std::string& password, const std::map<std::string, std::string>& tags) { callClientHook(&ClientProtocol::oper, server, client, username, password, tags); }
		void oper(const std::string& server, const std::string& client, const std::string& type, const std::map<std::string, std::string>& tags) { callServerHook(&ServerProtocol::oper, server, client, type, tags); }
		void sendServerNotice(const std::string& server, const std::string& type, const std::string& message, const std::map<std::string, std::string>& tags) { callServerHook(&ServerProtocol::sendServerNotice, server, type, message, tags); }
		void setMetadata(const std::string& server, const std::string& target, const std::string& key, const std::string& value, const std::map<std::string, std::string>& tags) { callServerHook(&ServerProtocol::setMetadata, server, target, key, value, tags); }
		void setXLine(const std::string& server, const std::string& source, const std::string& lineType, const std::string& mask, time_t duration, const std::string& reason, const std::map<std::string, std::string>& tags) { callServerHook(&ServerProtocol::setXLine, server, source, lineType, mask, duration, reason, tags); }
		void removeXLine(const std::string& server, const std::string& source, const std::string& lineType, const std::string& mask, const std::map<std::string, std::string>& tags) { callServerHook(&ServerProtocol::removeXLine, server, source, lineType, mask, tags); }
		void changeIdent(const std::string& server, const std::string& user, const std::string& ident, const std::map<std::string, std::string>& tags) { callServerHook(&ServerProtocol::changeIdent, server, user, ident, tags); }
		void changeHost(const std::string& server, const std::string& user, const std::string& host, const std::map<std::string, std::string>& tags) { callServerHook(&ServerProtocol::changeHost, server, user, host, tags); }
		void changeGecos(const std::string& server, const std::string& user, const std::string& gecos, const std::map<std::string, std::string>& tags) { callServerHook(&ServerProtocol::changeGecos, server, user, gecos, tags); }
		void sendWallops(const std::string& server, const std::string& source, const std::string& message, const std::map<std::string, std::string>& tags) { callEitherHook(&ClientProtocol::sendWallops, &ServerProtocol::sendWallops, server, source, message, tags); }
		void pushLine(const std::string& server, const std::string& user, const IRCMessage* line) { callServerHook(&ServerProtocol::pushLine, server, user, line); }
		void sendOtherData(const std::string& server, const std::string& client, const IRCMessage* line) { callClientHook(&ClientProtocol::sendOtherData, server, client, line); }
		void sendOtherData(const std::string& server, const IRCMessage* line) { callServerHook(&ServerProtocol::sendOtherData, server, line); }
		
		std::string addClient(const std::string& server, const std::string& nick, const std::string& ident, const std::string& host, const std::string& gecos, const std::map<std::string, std::string>& tags) { return callServerHook(&ServerProtocol::addClient, server, nick, ident, host, gecos, tags); }
		std::string addClient(const std::string& server, const std::string& nick, const std::string& ident, const std::string& gecos) { return callClientHook(&ClientProtocol::addClient, server, nick, ident, gecos); }
		void removeClient(const std::string& server, const std::string& client, const std::map<std::string, std::string>& tags) { callServerHook(&ServerProtocol::removeClient, server, client, tags); }
		void removeClient(const std::string& server, const std::string& client) { callEitherHook(&ClientProtocol::removeClient, server, client); }
		
		std::list<std::pair<std::string, bool>> activeServers(bool requestedByServerModule);
		std::list<std::pair<std::string, bool>> connectedServers(bool requestedByServerModule);
		std::string serverType(const std::string& server);
		
		std::list<std::string> networkServerList(const std::string& server) { return callServerHook(&ServerProtocol::networkServerList, server); }
		std::list<std::string> xLineTypes(const std::string& server) { return callServerHook(&ServerProtocol::xLineTypes, server); }
		std::list<std::string> xLineList(const std::string& server, const std::string& lineType) { return callServerHook(&ServerProtocol::xLineList, server, lineType); }
		time_t xLineExpiry(const std::string& server, const std::string& lineType, const std::string& mask) { return callServerHook(&ServerProtocol::xLineExpiry, server, lineType, mask); }
		std::string xLineReason(const std::string& server, const std::string& lineType, const std::string& mask) { return callServerHook(&ServerProtocol::xLineReason, server, lineType, mask); }
		
		std::set<std::string> chanTypes(const std::string& server) { return callEitherHook(&ClientProtocol::chanTypes, &ServerProtocol::chanTypes, server); }
		std::list<std::pair<ModeType, std::string>> allChanModes(const std::string& server) { return callEitherHook(&ClientProtocol::allChanModes, &ServerProtocol::allChanModes, server); }
		char prefixSymbol(const std::string& server, const std::string& mode) { return callEitherHook(&ClientProtocol::prefixSymbol, &ServerProtocol::prefixSymbol, server, mode); }
		std::pair<std::string, char> compareStatus(const std::string& server, const std::string& status0, const std::string& status1) { return callEitherHook(&ClientProtocol::compareStatus, &ServerProtocol::compareStatus, server, status0, status1); }
		std::pair<std::string, char> compareStatus(const std::string& server, const std::string& status0, char status1) { return callEitherHook(&ClientProtocol::compareStatus, &ServerProtocol::compareStatus, server, status0, status1); }
		std::pair<std::string, char> compareStatus(const std::string& server, char status0, const std::string& status1) { return callEitherHook(&ClientProtocol::compareStatus, &ServerProtocol::compareStatus, server, status1, status0); }
		std::pair<std::string, char> compareStatus(const std::string& server, char status0, char status1) { return callEitherHook(&ClientProtocol::compareStatus, &ServerProtocol::compareStatus, server, status0, status1); }
		
		std::list<std::string> chanList(const std::string& server) { return callServerHook(&ServerProtocol::chanList, server); }
		std::string chanTopic(const std::string& server, const std::string& channel) { return callEitherHook(&ClientProtocol::chanTopic, &ServerProtocol::chanTopic, server, channel); }
		std::string chanTopicSetter(const std::string& server, const std::string& channel) { return callEitherHook(&ClientProtocol::chanTopicSetter, &ServerProtocol::chanTopicSetter, server, channel); }
		time_t chanTimestamp(const std::string& server, const std::string& channel) { return callEitherHook(&ClientProtocol::chanTimestamp, &ServerProtocol::chanTimestamp, server, channel); }
		std::list<std::string> chanUsers(const std::string& server, const std::string& channel) { return callEitherHook(&ClientProtocol::chanUsers, &ServerProtocol::chanUsers, server, channel); }
		bool userInChan(const std::string& server, const std::string& channel, const std::string& user) { return callEitherHook(&ClientProtocol::userInChan, &ServerProtocol::userInChan, server, channel, user); }
		std::pair<std::string, char> userStatus(const std::string& server, const std::string& channel, const std::string& user) { return callEitherHook(&ClientProtocol::userStatus, &ServerProtocol::userStatus, server, channel, user); }
		bool userHasStatus(const std::string& server, const std::string& channel, const std::string& user, const std::string& status) { return callEitherHook(&ClientProtocol::userHasStatus, &ServerProtocol::userHasStatus, server, channel, user, status); }
		bool userHasStatus(const std::string& server, const std::string& channel, const std::string& user, char status) { return callEitherHook(&ClientProtocol::userHasStatus, &ServerProtocol::userHasStatus, server, channel, user, status); }
		bool userHasStatusOrGreater(const std::string& server, const std::string& channel, const std::string& user, const std::string& status) { return callEitherHook(&ClientProtocol::userHasStatusOrGreater, &ServerProtocol::userHasStatusOrGreater, server, channel, user, status); }
		bool userHasStatusOrGreater(const std::string& server, const std::string& channel, const std::string& user, char status) { return callEitherHook(&ClientProtocol::userHasStatusOrGreater, &ServerProtocol::userHasStatusOrGreater, server, channel, user, status); }
		std::map<std::string, std::string> chanModes(const std::string& server, const std::string& channel) { return callEitherHook(&ClientProtocol::chanModes, &ServerProtocol::chanModes, server, channel); }
		bool chanHasMode(const std::string& server, const std::string& channel, const std::string& mode) { return callEitherHook(&ClientProtocol::chanHasMode, &ServerProtocol::chanHasMode, server, channel, mode); }
		std::string chanModeParam(const std::string& server, const std::string& channel, const std::string& mode) { return callEitherHook(&ClientProtocol::chanModeParam, &ServerProtocol::chanModeParam, server, channel, mode); }
		std::list<std::string> chanListModeList(const std::string& server, const std::string& channel, const std::string& listMode) { return callServerHook(&ServerProtocol::chanListModeList, server, channel, listMode); }
		
		std::list<std::string> clientList(const std::string& server) { return callEitherHook(&ClientProtocol::clientList, &ServerProtocol::clientList, server); }
		std::string userNick(const std::string& server, const std::string& user) { return callEitherHook(&ClientProtocol::userNick, &ServerProtocol::userNick, server, user); }
		std::string userIdent(const std::string& server, const std::string& user) { return callEitherHook(&ClientProtocol::userIdent, &ServerProtocol::userIdent, server, user); }
		std::string userHost(const std::string& server, const std::string& user) { return callEitherHook(&ClientProtocol::userHost, &ServerProtocol::userHost, server, user); }
		std::string userGecos(const std::string& server, const std::string& user) { return callEitherHook(&ClientProtocol::userGecos, &ServerProtocol::userGecos, server, user); }
		std::map<std::string, std::string> userModes(const std::string& server, const std::string& user) { return callEitherHook(&ClientProtocol::userModes, &ServerProtocol::userModes, server, user); }
		bool userHasMode(const std::string& server, const std::string& user, const std::string& mode) { return callEitherHook(&ClientProtocol::userHasMode, &ServerProtocol::userHasMode, server, user, mode); }
		std::set<std::string> userChans(const std::string& server, const std::string& user) { return callEitherHook(&ClientProtocol::userChans, &ServerProtocol::userChans, server, user); }
		time_t userTimestamp(const std::string& server, const std::string& user) { return callServerHook(&ServerProtocol::userTimestamp, server, user); }
		time_t userNickTimestamp(const std::string& server, const std::string& user) { return callServerHook(&ServerProtocol::userNickTimestamp, server, user); }
		time_t userAwayTimestamp(const std::string& server, const std::string& user) { return callServerHook(&ServerProtocol::userAwayTimestamp, server, user); }
		
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
		RetVal callEitherHook(RetVal(ClientProtocol::*clientFunc)(Args&...), RetVal(ServerProtocol::*serverFunc)(Args&...), const std::string& server, Args&... args);
		template<typename RetVal, typename... Args>
		RetVal callClientHook(RetVal(ClientProtocol::*func)(Args&...), const std::string& server, Args&... args);
		template<typename RetVal, typename... Args>
		RetVal callServerHook(RetVal(ServerProtocol::*func)(Args&...), const std::string& server, Args&... args);
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
		ProtoLoadFailed(std::string&& desc) : description(std::forward<std::string>(desc)) {}
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