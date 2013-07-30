#pragma once
#include "../main.h"
#include "../modulemanager.h"
#include "../socketmanager.h"

class ServerProtocol {
	public:
		ServerProtocol(const std::string& name) : serverName(name) {}
		virtual ~ServerProtocol() {}
		void pointManagers(ModuleManager* mm, SocketManager* sm) { modmanager = mm; sockmanager = sm; }
		virtual void connectServer() {}
		virtual bool connected() { return false; }
		virtual bool shouldUnload() { return true; }
		virtual void disconnect() {}
		
		virtual void sendMsg(const std::string& server, const std::string& source, const std::string& target, const std::string& message, const std::map<std::string, std::string>& tags) {}
		virtual void sendNotice(const std::string& server, const std::string& source, const std::string& target, const std::string& message, const std::map<std::string, std::string>& tags) {}
		virtual void setMode(const std::string& server, const std::string& source, const std::string& target, const std::list<std::tuple<bool, std::string, std::string>> modes, const std::map<std::string, std::string>& tags) {}
		virtual void joinChan(const std::string& server, const std::string& client, const std::string& channel, const std::map<std::string, std::string>& tags) {}
		virtual void joinChanOver(const std::string& server, const std::string& client, const std::string& channel, const std::list<std::string>& statuses, time_t timestamp, const std::map<std::string, std::string>& tags) {}
		virtual void partChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void kickUser(const std::string& server, const std::string& source, const std::string& channel, const std::string& user, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void setTopic(const std::string& server, const std::string& source, const std::string& channel, const std::string& topic, const std::map<std::string, std::string>& tags) {}
		virtual void inviteUser(const std::string& server, const std::string& source, const std::string& channel, const std::string& user, const std::map<std::string, std::string>& tags) {}
		virtual void knock(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void changeNick(const std::string& server, const std::string& client, const std::string& nick, const std::map<std::string, std::string>& tags) {}
		virtual void sendPing(const std::string& server, const std::string& destServer, const std::map<std::string, std::string>& tags) {}
		virtual void setAway(const std::string& server, const std::string& client, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void setUnaway(const std::string& server, const std::string& client, const std::map<std::string, std::string>& tags) {}
		virtual void oper(const std::string& server, const std::string& client, const std::string& type, const std::map<std::string, std::string>& tags) {}
		virtual void sendServerNotice(const std::string& server, const std::string& type, const std::string& message, const std::map<std::string, std::string>& tags) {}
		virtual void setMetadata(const std::string& server, const std::string& target, const std::string& key, const std::string& value, const std::map<std::string, std::string>& tags) {}
		virtual void setXLine(const std::string& server, const std::string& source, const std::string& lineType, const std::string& mask, time_t duration, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void removeXLine(const std::string& server, const std::string& source, const std::string& lineType, const std::string& mask, const std::map<std::string, std::string>& tags) {}
		virtual void changeIdent(const std::string& server, const std::string& user, const std::string& ident, const std::map<std::string, std::string>& tags) {}
		virtual void changeHost(const std::string& server, const std::string& user, const std::string& host, const std::map<std::string, std::string>& tags) {}
		virtual void changeGecos(const std::string& server, const std::string& user, const std::string& gecos, const std::map<std::string, std::string>& tags) {}
		virtual void sendWallops(const std::string& server, const std::string& source, const std::string& message, const std::map<std::string, std::string>& tags) {}
		virtual void pushLine(const std::string& server, const std::string& user, const IRCMessage* line) {}
		virtual void sendOtherData(const std::string& server, const IRCMessage* line) {}
		
		virtual std::string addClient(const std::string& server, const std::string& nick, const std::string& ident, const std::string& host, const std::string& gecos) { return ""; }
		virtual void removeClient(const std::string& server, const std::string& client) {}
		
		virtual std::list<std::string> networkServerList(const std::string& server) { return std::list<std::string> (); }
		virtual std::list<std::string> xLineTypes(const std::string& server) { return std::list<std::string> (); }
		virtual std::list<std::string> xLineList(const std::string& server, const std::string& lineType) { return std::list<std::string> (); }
		virtual time_t xLineExpiry(const std::string& server, const std::string& lineType, const std::string& mask) { return std::list<std::string> (); }
		virtual std::string xLineReason(const std::string& server, const std::string& lineType, const std::string& mask) { return std::list<std::string> (); }
		
		virtual std::set<std::string> chanTypes(const std::string& server) { return std::set<std::string> (); }
		virtual std::list<std::pair<ModeType, std::string>> allChanModes(const std::string& server) { return std::list<std::pair<ModeType, std::string>> (); }
		virtual char prefixSymbol(const std::string& server, const std::string& mode) { return ' '; }
		virtual std::pair<std::string, char> compareStatus(const std::string& server, const std::string& status0, const std::string& status1) { return std::pair<std::string, char> ("", ' '); }
		virtual std::pair<std::string, char> compareStatus(const std::string& server, const std::string& status0, char status1) { return std::pair<std::string, char> ("", ' '); }
		virtual std::pair<std::string, char> compareStatus(const std::string& server, char status0, char status1) { return std::pair<std::string, char> ("", ' '); }
		
		virtual std::list<std::string> chanList(const std::string& server) { return std::list<std::string> (); }
		virtual std::string chanTopic(const std::string& server, const std::string& channel) { return ""; }
		virtual std::string chanTopicSetter(const std::string& server, const std::string& channel) { return ""; }
		virtual time_t chanTimestamp(const std::string& server, const std::string& channel) { return 0; }
		virtual std::list<std::string> chanUsers(const std::string& server, const std::string& channel) { return std::list<std::string> (); }
		virtual bool userInChan(const std::string& server, const std::string& channel, const std::string& user) { return false; }
		virtual std::pair<std::string, char> userStatus(const std::string& server, const std::string& channel, const std::string& user) { return std::pair<std::string, char> ("", ' '); }
		virtual bool userHasStatus(const std::string& server, const std::string& channel, const std::string& user, const std::string& status) { return false; }
		virtual bool userHasStatus(const std::string& server, const std::string& channel, const std::string& user, char status) { return false; }
		virtual bool userHasStatusOrGreater(const std::string& server, const std::string& channel, const std::string& user, const std::string& status) { return false; }
		virtual bool userHasStatusOrGreater(const std::string& server, const std::string& channel, const std::string& user, char status) { return false; }
		virtual std::map<std::string, std::string> chanModes(const std::string& server, const std::string& channel) { return std::map<std::string, std::string> (); }
		virtual bool chanHasMode(const std::string& server, const std::string& channel, const std::string& mode) { return false; }
		virtual std::string chanModeParam(const std::string& server, const std::string& channel, const std::string& mode) { return ""; }
		virtual std::list<std::string> chanListModeList(const std::string& server, const std::string& channel, const std::string& listMode) { return std::list<std::string> (); }
		
		virtual std::list<std::string> clientList(const std::string& server) { return std::list<std::string> (); }
		virtual std::string userNick(const std::string& server, const std::string& user) { return ""; }
		virtual std::string userIdent(const std::string& server, const std::string& user) { return ""; }
		virtual std::string userHost(const std::string& server, const std::string& user) { return ""; }
		virtual std::string userGecos(const std::string& server, const std::string& user) { return ""; }
		virtual std::map<std::string, std::string> userModes(const std::string& server, const std::string& user) { return std::map<std::string, std::string> (); }
		virtual bool userHasMode(const std::string& server, const std::string& user, const std::string& mode) { return false; }
		virtual std::set<std::string> userChans(const std::string& server, const std::string& user) { return std::set<std::string> (); }
		virtual time_t userTimestamp(const std::string& server, const std::string& user) { return 0; }
		virtual time_t userNickTimestamp(const std::string& server, const std::string& user) { return 0; }
		virtual time_t userAwayTimestamp(const std::string& server, const std::string& user) { return 0; }
	protected:
		const std::string serverName;
		std::shared_ptr<Socket> assignSocket(const std::string& type) { return sockmanager->getSocket(type); }
		template<typename... Args>
		void callHook(ActionType type, Args... args) { modmanager->callServerHook(type, serverName, args...); }
	private:
		ModuleManager* modmanager;
		SocketManager* sockmanager;
};