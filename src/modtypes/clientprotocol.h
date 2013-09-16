#pragma once
#include "../main.h"
#include "../modulemanager.h"
#include "../socketmanager.h"

class ClientProtocol {
	public:
		ClientProtocol(const std::string& name) : serverName(name) {}
		virtual ~ClientProtocol() {}
		void pointManagers(ModuleManager* mm, SocketManager* sm) { modmanager = mm; sockmanager = sm; }
		virtual void connectServer() {}
		virtual bool connected() { return false; }
		virtual bool shouldUnload() { return true; }
		virtual void disconnect() {}
		
		virtual void sendMsg(const std::string& server, const std::string& client, const std::string& target, const std::string& message, const std::map<std::string, std::string>& tags) {}
		virtual void sendNotice(const std::string& server, const std::string& client, const std::string& target, const std::string& message, const std::map<std::string, std::string>& tags) {}
		virtual void setMode(const std::string& server, const std::string& client, const std::string& target, const std::list<std::tuple<bool, std::string, std::string>>& modes, const std::map<std::string, std::string>& tags) {}
		virtual void joinChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& key, const std::map<std::string, std::string>& tags) {}
		virtual void partChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void kickUser(const std::string& server, const std::string& client, const std::string& channel, const std::string& user, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void setTopic(const std::string& server, const std::string& client, const std::string& channel, const std::string& topic, const std::map<std::string, std::string>& tags) {}
		virtual void inviteUser(const std::string& server, const std::string& client, const std::string& channel, const std::string& user, const std::map<std::string, std::string>& tags) {}
		virtual void knock(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void changeNick(const std::string& server, const std::string& client, const std::string& newNick, const std::map<std::string, std::string>& tags) {}
		virtual void sendPing(const std::string& server, const std::string& data, const std::map<std::string, std::string>& tags) {}
		virtual void setAway(const std::string& server, const std::string& client, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void setUnaway(const std::string& server, const std::string& client, const std::map<std::string, std::string>& tags) {}
		virtual void oper(const std::string& server, const std::string& client, const std::string& username, const std::string& password, const std::map<std::string, std::string>& tags) {}
		virtual void sendWallops(const std::string& server, const std::string& client, const std::string& message, const std::map<std::string, std::string>& tags) {}
		virtual void sendOtherData(const std::string& server, const std::string& client, const IRCMessage* line) {}
		
		virtual std::string addClient(const std::string& server, const std::string& nick, const std::string& ident, const std::string& gecos) { return ""; }
		virtual void removeClient(const std::string& server, const std::string& client) {}
		
		virtual std::set<std::string> chanTypes(const std::string& server) { return std::set<std::string> (); }
		virtual std::list<std::pair<ModeType, std::string>> allChanModes(const std::string& server) { return std::list<std::pair<ModeType, std::string>> (); }
		virtual char prefixSymbol(const std::string& server, const std::string& mode) { return ' '; }
		virtual std::pair<std::string, char> compareStatus(const std::string& server, const std::string& status0, const std::string& status1) { return std::pair<std::string, char> ("", ' '); }
		virtual std::pair<std::string, char> compareStatus(const std::string& server, const std::string& status0, char status1) { return std::pair<std::string, char> ("", ' '); }
		virtual std::pair<std::string, char> compareStatus(const std::string& server, char status0, char status1) { return std::pair<std::string, char> ("", ' '); }
		
		virtual std::string chanTopic(const std::string& server, const std::string& channel) { return ""; }
		virtual std::string chanTopicSetter(const std::string& server, const std::string& channel) { return ""; }
		virtual time_t chanTimestamp(const std::string& server, const std::string& channel) { return 0; }
		virtual std::list<std::string> chanUsers(const std::string& server, const std::string& channel) { return std::list<std::string> (); }
		virtual bool userInChan(const std::string& server, const std::string& channel, const std::string& user) { return false; }
		virtual std::pair<std::string, char> userStatus(const std::string& server, const std::string& channel, const std::string& user) { std::pair<std::string, char> ("", ' '); }
		virtual bool userHasStatus(const std::string& server, const std::string& channel, const std::string& user, const std::string& status) { return false; }
		virtual bool userHasStatus(const std::string& server, const std::string& channel, const std::string& user, char status) { return false; }
		virtual bool userHasStatusOrGreater(const std::string& server, const std::string& channel, const std::string& user, const std::string& status) { return false; }
		virtual bool userHasStatusOrGreater(const std::string& server, const std::string& channel, const std::string& user, char status) { return false; }
		virtual std::map<std::string, std::string> chanModes(const std::string& server, const std::string& channel) { return std::map<std::string, std::string> (); }
		virtual bool chanHasMode(const std::string& server, const std::string& channel, const std::string& mode) { return false; }
		virtual std::string chanModeParam(const std::string& server, const std::string& channel, const std::string& mode) { return ""; }
		
		virtual std::list<std::string> clientList(const std::string& server) { return std::list<std::string> (); }
		virtual std::string userNick(const std::string& server, const std::string& user) { return ""; }
		virtual std::string userIdent(const std::string& server, const std::string& user) { return ""; }
		virtual std::string userHost(const std::string& server, const std::string& user) { return ""; }
		virtual std::string userGecos(const std::string& server, const std::string& user) { return ""; }
		virtual std::map<std::string, std::string> userModes(const std::string& server, const std::string& user) { return std::map<std::string, std::string> (); }
		virtual bool userHasMode(const std::string& server, const std::string& user, const std::string& mode) { return false; }
		virtual std::set<std::string> userChans(const std::string& server, const std::string& user) { return std::set<std::string> (); }
	protected:
		const std::string serverName;
		std::shared_ptr<Socket> assignSocket(const std::string& type) { return sockmanager->getSocket(type); }
		template<typename... Args>
		void callHook(ActionType type, Args... args) { modmanager->callClientHook(type, serverName, args...); }
	private:
		ModuleManager* modmanager;
		SocketManager* sockmanager;
};