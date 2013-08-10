#pragma once
#include "../main.h"

class ServerProtocol {
	public:
		ServerProtocol(const std::string& name) : serverName(name) {}
		virtual ~ServerProtocol() {}
		virtual unsigned int apiVersion() = 0;
		void pointManagers(ModuleManager* mm, SocketManager* sm) { modmanager = mm; sockmanager = sm; }
		virtual void connectServer() {}
		virtual bool connected() { return false; }
		virtual bool shouldUnload() { return true; }
		virtual void disconnect(const std::string& reason) {}
		
		virtual void sendMsg(const std::string& source, const std::string& target, const std::string& message, const std::map<std::string, std::string>& tags) {}
		virtual void sendNotice(const std::string& source, const std::string& target, const std::string& message, const std::map<std::string, std::string>& tags) {}
		virtual void setMode(const std::string& source, const std::string& target, const std::list<std::tuple<bool, std::string, std::string>>& modes, const std::map<std::string, std::string>& tags) {}
		virtual void joinChan(const std::string& client, const std::string& channel, const std::map<std::string, std::string>& tags) {}
		virtual void joinChanOver(const std::string& client, const std::string& channel, const std::list<std::string>& statuses, time_t timestamp, const std::map<std::string, std::string>& tags) {}
		virtual void partChan(const std::string& client, const std::string& channel, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void kickUser(const std::string& source, const std::string& channel, const std::string& user, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void setTopic(const std::string& source, const std::string& channel, const std::string& topic, const std::map<std::string, std::string>& tags) {}
		virtual void inviteUser(const std::string& source, const std::string& channel, const std::string& user, const std::map<std::string, std::string>& tags) {}
		virtual void knock(const std::string& client, const std::string& channel, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void changeNick(const std::string& client, const std::string& nick, const std::map<std::string, std::string>& tags) {}
		virtual void sendPing(const std::string& destServer, const std::map<std::string, std::string>& tags) {}
		virtual void setAway(const std::string& client, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void setUnaway(const std::string& client, const std::map<std::string, std::string>& tags) {}
		virtual void oper(const std::string& client, const std::string& type, const std::map<std::string, std::string>& tags) {}
		virtual void sendServerNotice(const std::string& type, const std::string& message, const std::map<std::string, std::string>& tags) {}
		virtual void setMetadata(const std::string& target, const std::string& key, const std::string& value, const std::map<std::string, std::string>& tags) {}
		virtual void setXLine(const std::string& source, const std::string& lineType, const std::string& mask, time_t duration, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void removeXLine(const std::string& source, const std::string& lineType, const std::string& mask, const std::map<std::string, std::string>& tags) {}
		virtual void changeIdent(const std::string& user, const std::string& ident, const std::map<std::string, std::string>& tags) {}
		virtual void changeHost(const std::string& user, const std::string& host, const std::map<std::string, std::string>& tags) {}
		virtual void changeGecos(const std::string& user, const std::string& gecos, const std::map<std::string, std::string>& tags) {}
		virtual void sendWallops(const std::string& source, const std::string& message, const std::map<std::string, std::string>& tags) {}
		virtual void pushLine(const std::string& user, const IRCMessage* line) {}
		virtual void sendOtherData(const IRCMessage* line) {}
		
		virtual std::string addClient(const std::string& nick, const std::string& ident, const std::string& host, const std::string& gecos, const std::map<std::string, std::string>& tags) { return ""; }
		virtual void removeClient(const std::string& client, const std::map<std::string, std::string>& tags) {}
		
		virtual std::list<std::string> networkServerList() { return std::list<std::string> (); }
		virtual std::list<std::string> xLineTypes() { return std::list<std::string> (); }
		virtual std::list<std::string> xLineList(const std::string& lineType) { return std::list<std::string> (); }
		virtual time_t xLineExpiry(const std::string& lineType, const std::string& mask) { return 0; }
		virtual std::string xLineReason(const std::string& lineType, const std::string& mask) { return ""; }
		
		virtual std::set<char> chanTypes() { return std::set<char> (); }
		virtual std::list<std::pair<ModeType, std::string>> allChanModes() { return std::list<std::pair<ModeType, std::string>> (); }
		virtual ModeType chanModeType(const std::string& mode) { return MODE_NOPARAM; }
		virtual char prefixSymbol(const std::string& mode) { return ' '; }
		virtual std::pair<std::string, char> compareStatus(const std::string& status0, const std::string& status1) { return std::pair<std::string, char> ("", ' '); }
		virtual std::pair<std::string, char> compareStatus(const std::string& status0, char status1) { return std::pair<std::string, char> ("", ' '); }
		virtual std::pair<std::string, char> compareStatus(char status0, char status1) { return std::pair<std::string, char> ("", ' '); }
		
		virtual std::list<std::string> chanList() { return std::list<std::string> (); }
		virtual std::string chanTopic(const std::string& channel) { return ""; }
		virtual std::string chanTopicSetter(const std::string& channel) { return ""; }
		virtual time_t chanTimestamp(const std::string& channel) { return 0; }
		virtual std::list<std::string> chanUsers(const std::string& channel) { return std::list<std::string> (); }
		virtual bool userInChan(const std::string& channel, const std::string& user) { return false; }
		virtual std::pair<std::string, char> userStatus(const std::string& channel, const std::string& user) { return std::pair<std::string, char> ("", ' '); }
		virtual bool userHasStatus(const std::string& channel, const std::string& user, const std::string& status) { return false; }
		virtual bool userHasStatus(const std::string& channel, const std::string& user, char status) { return false; }
		virtual bool userHasStatusOrGreater(const std::string& channel, const std::string& user, const std::string& status) { return false; }
		virtual bool userHasStatusOrGreater(const std::string& channel, const std::string& user, char status) { return false; }
		virtual std::map<std::string, std::string> chanModes(const std::string& channel) { return std::map<std::string, std::string> (); }
		virtual bool chanHasMode(const std::string& channel, const std::string& mode) { return false; }
		virtual std::string chanModeParam(const std::string& channel, const std::string& mode) { return ""; }
		virtual std::list<std::string> chanListModeList(const std::string& channel, const std::string& listMode) { return std::list<std::string> (); }
		
		virtual std::list<std::string> clientList() { return std::list<std::string> (); }
		virtual std::string userNick(const std::string& user) { return ""; }
		virtual std::string userIdent(const std::string& user) { return ""; }
		virtual std::string userHost(const std::string& user) { return ""; }
		virtual std::string userGecos(const std::string& user) { return ""; }
		virtual std::string idFromNick(const std::string& nick) { return ""; }
		virtual std::list<std::pair<ModeType, std::string>> allUserModes() { return std::list<std::pair<ModeType, std::string>> (); }
		virtual ModeType userModeType(const std::string& mode) { return MODE_NOPARAM; }
		virtual std::map<std::string, std::string> userModes(const std::string& user) { return std::map<std::string, std::string> (); }
		virtual bool userHasMode(const std::string& user, const std::string& mode) { return false; }
		virtual std::string userModeParam(const std::string& user, const std::string& mode) { return ""; }
		virtual std::list<std::string> userListModeList(const std::string& user, const std::string& listMode) { return std::list<std::string> (); }
		virtual std::set<std::string> userChans(const std::string& user) { return std::set<std::string> (); }
		virtual time_t userTimestamp(const std::string& user) { return 0; }
		virtual time_t userNickTimestamp(const std::string& user) { return 0; }
		virtual time_t userAwayTimestamp(const std::string& user) { return 0; }
	protected:
		const std::string serverName;
		std::shared_ptr<Socket> assignSocket(const std::string& type) { return sockmanager->getSocket(type); }
		template<typename... Args>
		void callHook(ActionType type, Args... args) { modmanager->callServerHook(type, serverName, args...); }
	private:
		ModuleManager* modmanager;
		SocketManager* sockmanager;
};