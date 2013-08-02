#pragma once
#include "../main.h"

class ClientModule {
	public:
		ClientModule(std::shared_ptr<Module> base) : baseModule(base) {}
		virtual ~ClientModule() {}
		void loadManagerPointer(ModuleManager* mm, ServerManager* sm, SocketManager* sockm) { modmanager = mm; servmanager = sm; sockmanager = sockm; }
		
		virtual MsgAction onChanMsg(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& user, const std::string& message, const std::map<std::string, std::string>& tags) { return MSG_CONTINUE; }
		virtual MsgAction onUserMsg(const std::string& server, const std::string& client, const std::string& user, const std::string& message, const std::map<std::string, std::string>& tags) { return MSG_CONTINUE; }
		virtual MsgAction onChanNotice(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& user, const std::string& message, const std::map<std::string, std::string>& tags) { return MSG_CONTINUE; }
		virtual MsgAction onUserNotice(const std::string& server, const std::string& client, const std::string& user, const std::string& message, const std::map<std::string, std::string>& tags) { return MSG_CONTINUE; }
		virtual void onChanMode(const std::string& server, const std::string& client, const std::string& channel, const std::string& user, const std::list<std::tuple<ModeType, bool, std::string, std::string>>& modes, const std::map<std::string, std::string>& tags) {}
		virtual void onUserMode(const std::string& server, const std::string& client, const std::list<std::tuple<ModeType, bool, std::string, std::string>>& modes, const std::map<std::string, std::string>& tags) {}
		virtual void onJoin(const std::string& server, const std::string& client, const std::string& channel, const std::string& user, const std::map<std::string, std::string>& tags) {}
		virtual void onPart(const std::string& server, const std::string& client, const std::string& channel, const std::string& user, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void onQuit(const std::string& server, const std::string& client, const std::string& user, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void onKick(const std::string& server, const std::string& client, const std::string& channel, const std::string& userKicking, const std::string& userKicked, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void onTopic(const std::string& server, const std::string& client, const std::string& channel, const std::string& user, const std::string& topic, const std::map<std::string, std::string>& tags) {}
		virtual void onInvite(const std::string& server, const std::string& client, const std::string& channel, const std::string& user, const std::map<std::string, std::string>& tags) {}
		virtual void onKnock(const std::string& server, const std::string& client, const std::string& channel, const std::string& user, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void onNick(const std::string& server, const std::string& client, const std::string& user, const std::string& nick, const std::map<std::string, std::string>& tags) {}
		virtual void onPing(const std::string& server, const std::string& client, const std::string& data, const std::map<std::string, std::string>& tags) {}
		virtual void onPong(const std::string& server, const std::string& client, const std::string& data, const std::map<std::string, std::string>& tags) {}
		virtual void onAway(const std::string& server, const std::string& client, const std::string& user, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void onUnaway(const std::string& server, const std::string& client, const std::string& user, const std::map<std::string, std::string>& tags) {}
		virtual void onNumeric(const std::string& server, const std::string& client, const std::string& numeric, const std::vector<std::string>& params, const std::map<std::string, std::string>& tags) {}
		virtual void onWallops(const std::string& server, const std::string& client, const std::string& user, const std::string& message, const std::map<std::string, std::string>& tags) {}
		virtual void onCap(const std::string& server, const std::string& client, const std::string& subcmd, const std::list<std::string>& list, const std::map<std::string, std::string>& tags) {}
		virtual void onSelfConnect(const std::string& server, const std::string& client) {}
		virtual void onSelfQuit(const std::string& server, const std::string& client, const std::string& reason) {}
		virtual void onOtherData(const std::string& server, const std::string& client, const IRCMessage* message) {}
		
		virtual void onOutChanMsg(const std::string& server, const std::string& client, std::list<std::pair<std::string, char>>& channels, std::string& message, std::map<std::string, std::string>& tags) {}
		virtual void onOutUserMsg(const std::string& server, const std::string& client, std::list<std::string>& users, std::string& message, std::map<std::string, std::string>& tags) {}
		virtual void onOutChanNotice(const std::string& server, const std::string& client, std::list<std::pair<std::string, char>>& channels, std::string& message, std::map<std::string, std::string>& tags) {}
		virtual void onOutUserNotice(const std::string& server, const std::string& client, std::list<std::string>& users, std::string& message, std::map<std::string, std::string>& tags) {}
		virtual void onSendChanMsg(const std::string& server, const std::string& client, const std::list<std::pair<std::string, char>>& channels, char status, const std::string& message, const std::map<std::string, std::string>& tags) {}
		virtual void onSendUserMsg(const std::string& server, const std::string& client, const std::list<std::string>& users, const std::string& message, const std::map<std::string, std::string>& tags) {}
		virtual void onSendChanNotice(const std::string& server, const std::string& client, const std::list<std::pair<std::string, std::string>>& channels, char status, const std::string& message, const std::map<std::string, std::string>& tags) {}
		virtual void onSendUserNotice(const std::string& server, const std::string& client, const std::list<std::string>& users, const std::string& message, const std::map<std::string, std::string>& tags) {}
	protected:
		void sendMsg(const std::string& server, const std::string& client, const std::string& target, const std::string& message, const std::map<std::string, std::string>& tags) { servmanager->sendMsg(server, client, target, message, tags); }
		void sendNotice(const std::string& server, const std::string& client, const std::string& target, const std::string& message, const std::map<std::string, std::string>& tags) { servmanager->sendNotice(server, client, target, message, tags); }
		void setMode(const std::string& server, const std::string& client, const std::string& target, const std::list<std::tuple<bool, std::string, std::string>>& modes, const std::map<std::string, std::string>& tags) { servmanager->setMode(server, client, target, modes, tags); }
		void joinChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& key, const std::map<std::string, std::string>& tags) { servmanager->joinChan(server, client, channel, key, tags); }
		void partChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason, const std::map<std::string, std::string>& tags) { servmanager->partChan(server, client, channel, reason, tags); }
		void kickUser(const std::string& server, const std::string& client, const std::string& channel, const std::string& user, const std::string& reason, const std::map<std::string, std::string>& tags) { servmanager->kickUser(server, client, channel, user, reason, tags); }
		void setTopic(const std::string& server, const std::string& client, const std::string& channel, const std::string& topic, const std::map<std::string, std::string>& tags) { servmanager->setTopic(server, client, channel, topic, tags); }
		void inviteUser(const std::string& server, const std::string& client, const std::string& channel, const std::string& user, const std::map<std::string, std::string>& tags) { servmanager->inviteUser(server, client, channel, user, tags); }
		void knock(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason, const std::map<std::string, std::string>& tags) { servmanager->knock(server, client, channel, reason, tags); }
		void changeNick(const std::string& server, const std::string& client, const std::string& nick, const std::map<std::string, std::string>& tags) { servmanager->changeNick(server, client, nick, tags); }
		void sendPing(const std::string& server, const std::string& client, const std::string& data, const std::map<std::string, std::string>& tags) { servmanager->sendPing(server, client, data, tags); }
		void setAway(const std::string& server, const std::string& client, const std::string& reason, const std::map<std::string, std::string>& tags) { servmanager->setAway(server, client, reason, tags); }
		void setUnaway(const std::string& server, const std::string& client, const std::map<std::string, std::string>& tags) { servmanager->setUnaway(server, client, tags); }
		void oper(const std::string& server, const std::string& client, const std::string& username, const std::string& password, const std::map<std::string, std::string>& tags) { servmanager->oper(server, client, username, password, tags); }
		void sendWallops(const std::string& server, const std::string& client, const std::string& message, const std::map<std::string, std::string>& tags) { servmanager->sendWallops(server, client, message, tags); }
		void sendOtherData(const std::string& server, const std::string& client, const IRCMessage* line) { servmanager->sendOtherData(server, client, line); }
		
		std::string addClient(const std::string& server, const std::string& nick, const std::string& ident, const std::string& gecos) { return servmanager->addClient(server, nick, ident, gecos); }
		void removeClient(const std::string& server, const std::string& client) { servmanager->removeClient(server, client); }
		
		std::list<std::pair<std::string, bool>> activeServers() { return servmanager->activeServers(false); }
		std::list<std::pair<std::string, bool>> connectedServers() { return servmanager->connectedServers(false); }
		std::string serverType(const std::string& server) { return servmanager->serverType(server); }
		
		std::set<std::string> chanTypes(const std::string& server) { return servmanager->chanTypes(server); }
		std::list<std::pair<ModeType, std::string>> allChanModes(const std::string& server) { return servmanager->allChanModes(server); }
		char prefixSymbol(const std::string& server, const std::string& mode) { return servmanager->prefixSymbol(server, mode); }
		std::pair<std::string, char> compareStatus(const std::string& server, const std::string& status0, const std::string& status1) { return servmanager->compareStatus(server, status0, status1); }
		std::pair<std::string, char> compareStatus(const std::string& server, const std::string& status0, char status1) { return servmanager->compareStatus(server, status0, status1); }
		std::pair<std::string, char> compareStatus(const std::string& server, char status0, const std::string& status1) { return servmanager->compareStatus(server, status1, status0); }
		std::pair<std::string, char> compareStatus(const std::string& server, char status0, char status1) { return servmanager->compareStatus(server, status0, status1); }
		
		std::string chanTopic(const std::string& server, const std::string& channel) { return servmanager->chanTopic(server, channel); }
		std::string chanTopicSetter(const std::string& server, const std::string& channel) { return servmanager->chanTopicSetter(server, channel); }
		time_t chanTimestamp(const std::string& server, const std::string& channel) { return servmanager->chanTimestamp(server, channel); }
		std::list<std::string> chanUsers(const std::string& server, const std::string& channel) { return servmanager->chanUsers(server, channel); }
		bool userInChan(const std::string& server, const std::string& channel, const std::string& user) { return servmanager->userInChan(server, channel, user); }
		std::pair<std::string, char> userStatus(const std::string& server, const std::string& channel, const std::string& user) { return servmanager->userStatus(server, channel, user); }
		bool userHasStatus(const std::string& server, const std::string& channel, const std::string& user, const std::string& status) { return servmanager->userHasStatus(server, channel, user, status); }
		bool userHasStatus(const std::string& server, const std::string& channel, const std::string& user, char status) { return servmanager->userHasStatus(server, channel, user, status); }
		bool userHasStatusOrGreater(const std::string& server, const std::string& channel, const std::string& user, const std::string& status) { return servmanager->userHasStatusOrGreater(server, channel, user, status); }
		bool userHasStatusOrGreater(const std::string& server, const std::string& channel, const std::string& user, char status) { return servmanager->userHasStatusOrGreater(server, channel, user, status); }
		std::map<std::string, std::string> chanModes(const std::string& server, const std::string& channel) { return servmanager->chanModes(server, channel); }
		bool chanHasMode(const std::string& server, const std::string& channel, const std::string& mode) { return servmanager->chanHasMode(server, channel, mode); }
		std::string chanModeParam(const std::string& server, const std::string& channel, const std::string& mode) { return servmanager->chanModeParam(server, channel, mode); }
		std::list<std::string> chanListModeList(const std::string& server, const std::string& channel, const std::string& mode) { return servmanager->chanListModeList(server, channel, mode); }
		
		std::list<std::string> clientList(const std::string& server) { return servmanager->clientList(server); }
		std::string userNick(const std::string& server, const std::string& user) { return servmanager->userNick(server, user); }
		std::string userIdent(const std::string& server, const std::string& user) { return servmanager->userIdent(server, user); }
		std::string userHost(const std::string& server, const std::string& user) { return servmanager->userHost(server, user); }
		std::string userGecos(const std::string& server, const std::string& user) { return servmanager->userGecos(server, user); }
		std::map<std::string, std::string> userModes(const std::string& server, const std::string& user) { return servmanager->userModes(server, user); }
		bool userHasMode(const std::string& server, const std::string& user, const std::string& mode) { return servmanager->userHasMode(server, user, mode); }
		std::string userModeParam(const std::string& server, const std::string& user, const std::string& mode) { return servmanager->userModeParam(server, user, mode); }
		std::list<std::string> userListModeList(const std::string& server, const std::string& user, const std::string& mode) { return servmanager->userListModeList(server, user, mode); }
		std::set<std::string> userChans(const std::string& server, const std::string& user) { return servmanager->userChans(server, user); }
	private:
		std::shared_ptr<Module> baseModule;
		ModuleManager* modmanager;
		ServerManager* servmanager;
		SocketManager* sockmanager;
};