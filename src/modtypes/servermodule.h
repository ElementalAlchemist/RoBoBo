#pragma once
#include "../main.h"
#include "basemodule.h"

class ServerModule {
	public:
		ServerModule(std::shared_ptr<Module> base) : baseModule(base) {}
		void loadManagerPointer(ModuleManager* mm, ServerManager* sm, SocketManager* sockm) { modmanager = mm; servmanager = sm; sockmanager = sockm; }
		
		virtual MsgAction onChanMsg(const std::string& server, const std::string& channel, char status, const std::string& user, const std::string& message, const std::map<std::string, std::string>& tags) {}
		virtual MsgAction onUserMsg(const std::string& server, const std::string& client, const std::string& user, const std::string& message, const std::map<std::string, std::string>& tags) {}
		virtual MsgAction onChanNotice(const std::string& server, const std::string& channel, char status, const std::string& user, const std::string& message, const std::map<std::string, std::string>& tags) {}
		virtual MsgAction onUserNotice(const std::string& server, const std::string& client, const std::string& user, const std::string& message, const std::map<std::string, std::string>& tags) {}
		virtual void onChanMode(const std::string& server, const std::string& channel, const std::list<std::tuple<ModeType, bool, std::string, std::string>> modes, const std::map<std::string, std::string>& tags) {}
		virtual void onUserMode(const std::string& server, const std::string& user, const std::list<std::tuple<ModeType, bool, std::string, std::string>> modes, const std::map<std::string, std::string>& tags) {}
		virtual void onJoin(const std::string& server, const std::string& channel, const std::string& user, const std::map<std::string, std::string>& tags) {}
		virtual void onPart(const std::string& server, const std::string& channel, const std::string& user, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void onKick(const std::string& server, const std::string& channel, const std::string& userKicking, const std::string& userKicked, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void onTopic(const std::string& server, const std::string& channel, const std::string& user, const std::string& topic, const std::map<std::string, std::string>& tags) {}
		virtual void onInvite(const std::string& server, const std::string& channel, const std::string& userInviting, const std::string& userInvited, const std::map<std::string, std::string>& tags) {}
		virtual void onKnock(const std::string& server, const std::string& channel, const std::string& user, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void onNick(const std::string& server, const std::string& user, const std::string& nick, const std::map<std::string, std::string>& tags) {}
		virtual void onPing(const std::string& server, const std::string& pingingServer, const std::map<std::string, std::string>& tags) {}
		virtual void onPong(const std::string& server, const std::string& pongingServer, const std::map<std::string, std::string>& tags) {}
		virtual void onAway(const std::string& server, const std::string& user, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void onUnaway(const std::string& server, const std::string& user, const std::map<std::string, std::string>& tags) {}
		virtual void onChanCreate(const std::string& server, const std::string& channel, const std::list<std::tuple<ModeType, bool, std::string, std::string>>& modes, const std::list<std::pair<std::string, std::list<std::string>>>& users, const std::map<std::string, std::string>& tags) {}
		virtual void onChanDestroy(const std::string& server, const std::string& channel) {}
		virtual void onRequestStats(const std::string& server, const std::string& user, const std::string& type, const std::map<std::string, std::string>& tags) {}
		virtual void onRequestMOTD(const std::string& server, const std::string& user, const std::map<std::string, std::string>& tags) {}
		virtual void onRequestTime(const std::string& server, const std::string& user, const std::map<std::string, std::string>& tags) {}
		virtual void onRequestAdmin(const std::string& server, const std::string& user, const std::map<std::string, std::string>& tags) {}
		virtual void onRequestInfo(const std::string& server, const std::string& user, const std::map<std::string, std::string>& tags) {}
		virtual void onServerNotice(const std::string& server, const std::string& sourceServer, const std::string& type, const std::string& notice, const std::map<std::string, std::string>& tags) {}
		virtual void onChanMetadata(const std::string& server, const std::string& channel, const std::string& key, const std::string& value, const std::map<std::string, std::string>& tags) {}
		virtual void onUserMetadata(const std::string& server, const std::string& user, const std::string& key, const std::string& value, const std::map<std::string, std::string>& tags) {}
		virtual void onAddXLine(const std::string& server, const std::string& lineType, const std::string& mask, time_t expiry, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void onRemoveXLine(const std::string& server, const std::string& lineType, const std::string& mask, const std::map<std::string, std::string>& tags) {}
		virtual void onUserConnect(const std::string& server, const std::string& user, const std::map<std::string, std::string>& tags) {}
		virtual void onUserQuit(const std::string& server, const std::string& user, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void onUserSelfConnect(const std::string& server, const std::string& user, const std::map<std::string, std::string>& tags) {}
		virtual void onUserSelfQuit(const std::string& server, const std::string& user, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void onChangeIdent(const std::string& server, const std::string& user, const std::string& ident, const std::map<std::string, std::string>& tags) {}
		virtual void onChangeHost(const std::string& server, const std::string& user, const std::string& host, const std::map<std::string, std::string>& tags) {}
		virtual void onChangeGecos(const std::string& server, const std::string& user, const std::string& gecos, const std::map<std::string, std::string>& tags) {}
		virtual void onWallops(const std::string& server, const std::string& user, const std::string& message, const std::map<std::string, std::string>& tags) {}
		virtual void onServerConnect(const std::string& server, const std::string& connectingServer, const std::map<std::string, std::string>& tags) {}
		virtual void onServerQuit(const std::string& server, const std::string& quittingServer, const std::string& reason, const std::map<std::string, std::string>& tags) {}
		virtual void onCapab(const std::string& server, const std::vector<std::string>& data, const std::map<std::string, std::string>& tags) {}
		virtual void onBurst(const std::string& server, const std::map<std::string, std::string>& tags) {}
		virtual void onEndBurst(const std::string& server, const std::map<std::string, std::string>& tags) {}
		virtual void onSelfConnect(const std::string& server) {}
		virtual void onSelfQuit(const std::string& server, const std::string& reason) {}
		virtual void onOtherData(const std::string& server, const IRCMessage* message) {}
		
		virtual void onOutChanMsg(const std::string& server, std::string& client, std::list<std::pair<std::string, char>>& channels, std::string& message, std::map<std::string, std::string>& tags) {}
		virtual void onOutUserMsg(const std::string& server, std::string& client, std::list<std::string>& users, std::string& message, std::map<std::string, std::string>& tags) {}
		virtual void onOutChanNotice(const std::string& server, std::string& client, std::list<std::pair<std::string, char>>& channels, std::string& message, std::map<std::string, std::string>& tags) {}
		virtual void onOutUserNotice(const std::string& server, std::string& client, std::list<std::string>& users, std::string& message, std::map<std::string, std::string>& tags) {}
		virtual void onSendChanMsg(const std::string& server, const std::string& client, const std::list<std::pair<std::string, char>>& channels, const std::string& message, const std::map<std::string, std::string>& tags) {}
		virtual void onSendUserMsg(const std::string& server, const std::string& client, const std::list<std::string>& users, const std::string& message, const std::map<std::string, std::string>& tags) {}
		virtual void onSendChanNotice(const std::string& server, const std::string& client, const std::list<std::pair<std::string, char>>& channels, const std::string& message, const std::map<std::string, std::string>& tags) {}
		virtual void onSendUserNotice(const std::string& server, const std::string& client, const std::list<std::string>& users, const std::string& message, const std::map<std::string, std::string>& tags) {}
	protected:
		// TODO: Define all of these once the ServerManager is set up
		void sendMsg(const std::string& server, const std::string& source, const std::string& target, const std::string& message, const std::map<std::string, std::string>& tags);
		void sendNotice(const std::string& server, const std::string& source, const std::string& target, const std::string& message, const std::map<std::string, std::string>& tags);
		void setMode(const std::string& server, const std::string& source, const std::string& target, const std::list<std::tuple<bool, std::string, std::string>> modes, const std::map<std::string, std::string>& tags);
		void joinChan(const std::string& server, const std::string& client, const std::string& channel, const std::map<std::string, std::string>& tags);
		void joinChanOver(const std::string& server, const std::string& client, const std::string& channel, const std::list<std::string>& statuses, time_t timestamp, const std::map<std::string, std::string>& tags);
		void partChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason, const std::map<std::string, std::string>& tags);
		void kickUser(const std::string& server, const std::string& source, const std::string& channel, const std::string& user, const std::string& reason, const std::map<std::string, std::string>& tags);
		void setTopic(const std::string& server, const std::string& source, const std::string& channel, const std::string& topic, const std::map<std::string, std::string>& tags);
		void inviteUser(const std::string& server, const std::string& source, const std::string& channel, const std::string& user, const std::map<std::string, std::string>& tags);
		void knock(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason, const std::map<std::string, std::string>& tags);
		void changeNick(const std::string& server, const std::string& client, const std::string& nick, const std::map<std::string, std::string>& tags);
		void sendPing(const std::string& server, const std::string& destServer, const std::string& data, const std::map<std::string, std::string>& tags);
		void setAway(const std::string& server, const std::string& client, const std::string& reason, const std::map<std::string, std::string>& tags);
		void setUnaway(const std::string& server, const std::string& client, const std::map<std::string, std::string>& tags);
		void oper(const std::string& server, const std::string& client, const std::string& type, const std::map<std::string, std::string>& tags);
		void sendServerNotice(const std::string& server, const std::string& type, const std::string& message, const std::map<std::string, std::string>& tags);
		void setMetadata(const std::string& server, const std::string& target, const std::string& key, const std::string& value, const std::map<std::string, std::string>& tags);
		void setXLine(const std::string& server, const std::string& source, const std::string& lineType, const std::string& mask, time_t duration, const std::string& reason, const std::map<std::string, std::string>& tags);
		void removeXLine(const std::string& server, const std::string& source, const std::string& lineType, const std::string& mask, const std::map<std::string, std::string>& tags);
		void changeIdent(const std::string& server, const std::string& user, const std::string& ident, const std::map<std::string, std::string>& tags);
		void changeHost(const std::string& server, const std::string& user, const std::string& host, const std::map<std::string, std::string>& tags);
		void changeGecos(const std::string& server, const std::string& user, const std::string& gecos, const std::map<std::string, std::string>& tags);
		void sendWallops(const std::string& server, const std::string& source, const std::string& message, const std::map<std::string, std::string>& tags);
		void pushLine(const std::string& server, const std::string& user, const IRCMessage* line);
		void sendOtherData(const std::string& server, const IRCMessage* message);
		
		std::string addClient(const std::string& server, const std::string& nick, const std::string& ident, const std::string& host, const std::string& gecos);
		void removeClient(const std::string& server, const std::string& client);
		
		std::list<std::pair<std::string, bool>> activeServers();
		std::list<std::pair<std::string, bool>> connectedServers();
		std::string serverType(const std::string& server);
		
		std::list<std::string> networkServerList(const std::string& server);
		std::list<std::string> xLineTypes(const std::string& server);
		std::list<std::string> xLineList(const std::string& server, const std::string& lineType);
		time_t xLineExpiry(const std::string& server, const std::string& lineType, const std::string& mask);
		std::string xLineReason(const std::string& server, const std::string& lineType, const std::string& mask);
		
		std::set<std::string> chanTypes(const std::string& server);
		std::list<std::pair<ModeType, std::string>> allChanModes(const std::string& server);
		char prefixSymbol(const std::string& server, const std::string& mode);
		std::pair<std::string, char> compareStatus(const std::string& server, const std::string& status0, const std::string& status1);
		std::pair<std::string, char> compareStatus(const std::string& server, const std::string& status0, char status1);
		std::pair<std::string, char> compareStatus(const std::string& server, char status0, const std::string& status1);
		std::pair<std::string, char> compareStatus(const std::string& server, char status0, char status1);
		
		std::list<std::string> chanList(const std::string& server);
		std::string chanTopic(const std::string& server, const std::string& channel);
		std::string chanTopicSetter(const std::string& server, const std::string& channel);
		time_t chanTimestamp(const std::string& server, const std::string& channel);
		std::list<std::string> chanUsers(const std::string& server, const std::string& channel);
		bool userInChan(const std::string& server, const std::string& channel, const std::string& user);
		std::pair<std::string, char> userStatus(const std::string& server, const std::string& channel, const std::string& user);
		bool userHasStatus(const std::string& server, const std::string& channel, const std::string& user, const std::string& status);
		bool userHasStatus(const std::string& server, const std::string& channel, const std::string& user, char status);
		bool userHasStatusOrGreater(const std::string& server, const std::string& channel, const std::string& user, const std::string& status);
		bool userHasStatusOrGreater(const std::string& server, const std::string& channel, const std::string& user, char status);
		std::map<std::string, std::string> chanModes(const std::string& server, const std::string& channel);
		bool chanHasMode(const std::string& server, const std::string& channel, const std::string& mode);
		std::string chanModeParam(const std::string& server, const std::string& channel, const std::string& mode);
		std::list<std::string> chanListModeList(const std::string& server, const std::string& channel, const std::string& listMode);
		
		std::list<std::string> clientList(const std::string& server);
		std::string userNick(const std::string& server, const std::string& user);
		std::string userIdent(const std::string& server, const std::string& user);
		std::string userHost(const std::string& server, const std::string& user);
		std::string userGecos(const std::string& server, const std::string& user);
		std::map<std::string, std::string> userModes(const std::string& server, const std::string& user);
		bool userHasMode(const std::string& server, const std::string& user, const std::string& mode);
		std::set<std::string> userChans(const std::string& server, const std::string& user);
		time_t userTimestamp(const std::string& server, const std::string& user);
		time_t userNickTimestamp(const std::string& server, const std::string& user);
		time_t userAwayTimestamp(const std::string& server, const std::string& user);
	private:
		std::shared_ptr<Module> baseModule;
		ModuleManager* modmanager;
		ServerManager* servmanager;
		SocketManager* sockmanager;
};