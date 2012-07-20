#pragma once
#include "main.h"

#include "socket.h"

class Base;

class Protocol {
	public:
		Protocol(std::string server, std::map<std::string, std::string> conf, std::string workDir, bool dumpLogs, unsigned short debug, Base* botptr);
		virtual ~Protocol();
		virtual unsigned int apiVersion() = 0;
		virtual void connectServer();
		virtual void disconnectServer(std::string reason);
		virtual bool isConnected();
		virtual bool deadServer();
		virtual bool isClient();
		void endDebug();
		
		virtual void sendPrivMsg(const std::string& client, const std::string& target, const std::string& message) {}
		virtual void sendNotice(const std::string& client, const std::string& target, const std::string& message) {}
		virtual void sendCTCP(const std::string& client, const std::string& target, const std::string& ctcp, const std::string& params) {}
		virtual void sendCTCPReply(const std::string& client, const std::string& target, const std::string& ctcp, const std::string& params) {}
		virtual void setMode(const std::string& client, const std::string& target, const std::list<std::string>& setModes, const std::list<std::string>& remModes) {}
		virtual void setSNOmask(const std::string& client, bool add, char snomask) {}
		virtual void joinChan(const std::string& client, const std::string& channel, const std::string& key) {}
		virtual void partChan( const std::string& client, const std::string& channel, const std::string& reason) {}
		virtual void kickUser(const std::string& client, const std::string& channel, const std::string& user, const std::string& reason) {}
		virtual std::string addClient(std::string& nick, std::string& ident, std::string& host, std::string& gecos) { return ""; }
		virtual void removeClient(const std::string& client) {}
		virtual void setTopic(const std::string& client, const std::string& channel, const std::string& topic) {}
		virtual void inviteUser(const std::string& client, const std::string& channel, const std::string& user) {}
		virtual void knockOnChannel(const std::string& client, const std::string& channel, const std::string& reason) {}
		virtual void changeNick(const std::string& user, const std::string& newNick) {}
		virtual void sendPing(const std::string& remoteServer) {}
		virtual void operUp(const std::string& client, const std::string& usernameOrType, const std::string& password) {}
		virtual void sendServerNotice(char snomask, const std::string& message) {}
		virtual void setMetadata(const std::string& target, const std::string& key, const std::string& value) {}
		virtual void setXLine(const std::string& client, const std::string& lineType, const std::string& mask, time_t duration, const std::string& reason) {}
		virtual void remXLine(const std::string& client, const std::string& lineType, const std::string& mask) {}
		virtual void changeIdent(const std::string& user, const std::string& newIdent) {}
		virtual void changeHost(const std::string& user, const std::string& newHost) {}
		virtual void changeGecos(const std::string& user, const std::string& newGecos) {}
		virtual void sendWallops(const std::string& client, const std::string& message) {}
		virtual void sendOtherData(const std::string& client, const std::string& line) {}
		
		virtual std::list<std::string> networkServerList() { return std::list<std::string> (); }
		virtual std::list<std::string> xLineTypes() { return std::list<std::string> (); }
		virtual std::list<std::string> xLineList(const std::string& lineType) { return std::list<std::string> (); }
		virtual time_t xLineExpiry(const std::string& lineType, const std::string& mask) { return 0; }
		virtual std::string xLineReason(const std::string& lineType, const std::string& mask) { return ""; }
		virtual std::list<std::string> chanListModes() { return std::list<std::string> (); }
		virtual std::list<std::string> chanParamModes() { return std::list<std::string> (); }
		virtual std::list<std::string> chanNoParamModes() { return std::list<std::string> (); }
		virtual std::list<std::pair<std::string, char>> chanPrefixes() { return std::list<std::pair<std::string, char>> (); }
		virtual std::pair<std::string, char> compareStatus(const std::string& status0, const std::string& status1) { return std::pair<std::string, char> ("", ' '); }
		virtual std::pair<std::string, char> compareStatus(const std::string& status0, char status1) { return std::pair<std::string, char> ("", ' '); }
		virtual std::pair<std::string, char> compareStatus(char status0, char status1) { return std::pair<std::string, char> ("", ' '); }
		
		virtual std::string chanTopic(const std::string& channel) { return ""; }
		virtual time_t chanTimestamp(const std::string& channel) { return 0; }
		virtual std::set<std::string> chanUsers(const std::string& channel) { return std::set<std::string> (); }
		virtual bool userInChan(const std::string& channel, const std::string& user) { return false; }
		virtual std::pair<std::string, char> userStatus(const std::string& channel, const std::string& user) { return std::pair<std::string, char> ("", ' '); }
		virtual bool userHasStatus(const std::string& channel, const std::string& user, const std::string& status) { return false; }
		virtual bool userHasStatus(const std::string& channel, const std::string& user, char status) { return false; }
		virtual bool userHasStatusOrGreater(const std::string& channel, const std::string& user, const std::string& status) { return false; }
		virtual bool userHasStatusOrGreater(const std::string& channel, const std::string& user, char status) { return false; }
		virtual std::list<std::string> chanModes(const std::string& channel) { return std::list<std::string> (); }
		virtual std::list<std::string> chanListModeList(const std::string& channel, const std::string& listMode) { return std::list<std::string> (); }
		virtual bool chanHasMode(const std::string& channel, const std::string& mode) { return false; }
		virtual std::string chanModeParam(const std::string& channel, const std::string& mode) { return ""; }
		
		virtual std::list<std::string> clientList() { return std::list<std::string> (); }
		virtual std::string clientNick(const std::string& client) { return ""; }
		virtual std::string userIdent(const std::string& user) { return ""; }
		virtual std::string userHost(const std::string& user) { return ""; }
		virtual std::string userGecos(const std::string& user) { return ""; }
		virtual std::list<std::string> userModes(const std::string& user) { return std::list<std::string> (); }
		virtual std::list<char> userSNOmasks(const std::string& user) { return std::list<char> (); }
		virtual std::list<std::string> userChans(const std::string& user) { return std::list<std::string> (); }
		virtual time_t userTimestamp(const std::string& user) { return 0; }
		virtual time_t userNickTimestamp(const std::string& user) { return 0; }
		
		virtual void processedOutChanMsg(const std::string& client, const std::string& channel, char status, const std::string& message) {}
		virtual void processedOutUserMsg(const std::string& client, const std::string& nick, const std::string& message) {}
		virtual void processedOutChanNotice(const std::string& client, const std::string& channel, char status, const std::string& message) {}
		virtual void processedOutUserNotice(const std::string& client, const std::string& nick, const std::string& message) {}
		virtual void processedOutChanCTCP(const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params) {}
		virtual void processedOutUserCTCP(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) {}
		virtual void processedOutChanCTCPReply(const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params) {}
		virtual void processedOutUserCTCPReply(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) {}
	protected:
		std::string serverName, workingDir;
		std::map<std::string, std::string> config;
		bool log;
		unsigned short debugLevel;
		std::shared_ptr<Socket> assignSocket(std::string socketType);
		
		void callChanMsgHook(const std::string& client, const std::string& channel, char status, const std::string& nick, const std::string& message) { bot->modChanMsgHook(serverName, client, channel, status, nick, message); }
		void callUserMsgHook(const std::string& client, const std::string& nick, const std::string& message) { bot->modUserMsgHook(serverName, client, nick, message); }
		void callChanNoticeHook(const std::string& client, const std::string& channel, char status, const std::string& nick, const std::string& message) { bot->modChanNoticeHook(serverName, client, channel, status, nick, message); }
		void callUserNoticeHook(const std::string& client, const std::string& nick, const std::string& message) { bot->modUserNoticeHook(serverName, client, nick, message); }
		void callChanCTCPHook(const std::string& client, const std::string& channel, char status, const std::string& nick, const std::string& ctcp, const std::string& params) { bot->modChanCTCPHook(serverName, client, channel, status, nick, ctcp, params); }
		void callUserCTCPHook(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) { bot->modUserCTCPHook(serverName, client, nick, ctcp, params); }
		void callChanCTCPReplyHook(const std::string& client, const std::string& channel, char status, const std::string& nick, const std::string& ctcp, const std::string& params) { bot->modChanCTCPReplyHook(serverName, client, channel, status, nick, ctcp, params); }
		void callUserCTCPReplyHook(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) { bot->modUserCTCPReplyHook(serverName, client, nick, ctcp, params); }
		void callChanModeHook(const std::string& channel, bool add, const std::string& mode) { bot->modChanModeHook(serverName, channel, add, mode); }
		void callUserModeHook(const std::string& nick, bool add, const std::string& mode) { bot->modUserModeHook(serverName, nick, add, mode); }
		void callUserSNOmaskHook(const std::string& nick, bool add, char snomask) { bot->modUserSNOmaskHook(serverName, nick, add, snomask); }
		void callChanJoinHook(const std::string& channel, const std::string& user) { bot->modChanJoinHook(serverName, channel, user); }
		void callChanPartHook(const std::string& channel, const std::string& user, const std::string& reason) { bot->modChanPartHook(serverName, channel, user, reason); }
		void callChanKickHook(const std::string& channel, const std::string& kicker, const std::string& kickee, const std::string& reason) { bot->modChanKickHook(serverName, channel, kicker, kickee, reason); }
		void callChanTopicHook(const std::string& channel, const std::string& nick, const std::string& topic) { bot->modChanTopicHook(serverName, channel, nick, topic); }
		void callChanInviteHook(const std::string& channel, const std::string& inviter, const std::string& invitee) { bot->modChanInviteHook(serverName, channel, inviter, invitee); }
		void callChanKnockHook(const std::string& channel, const std::string& nick, const std::string& reason) { bot->modChanKnockHook(serverName, channel, nick, reason); }
		void callUserNickHook(const std::string& oldNick, const std::string& newNick) { bot->modUserNickHook(serverName, oldNick, newNick); }
		void callServerPingHook(const std::string& sourceServer) { bot->modServerPingHook(serverName, sourceServer); }
		void callServerPongHook(const std::string& sourceServer) { bot->modServerPongHook(serverName, sourceServer); }
		void callNumericHook(const std::string& numeric, const std::vector<std::string>& data) { bot->modNumericHook(serverName, numeric, data); }
		void callUserOperHook(const std::string& nick, const std::string& operType) { bot->modUserOperHook(serverName, nick, operType); }
		void callServerNoticeHook(char snomask, const std::string& message) { bot->modServerNoticeHook(serverName, snomask, message); }
		void callMetadataHook(const std::string& target, const std::string& key, const std::string& value) { bot->modMetadataHook(serverName, target, key, value); }
		void callXLineAddHook(const std::string& lineType, const std::string& mask) { bot->modXLineAddHook(serverName, lineType, mask); }
		void callXLineRemoveHook(const std::string& lineType, const std::string& mask) { bot->modXLineRemoveHook(serverName, lineType, mask); }
		void callUserConnectHook(const std::string& nick) { bot->modUserConnectHook(serverName, nick); }
		void callUserQuitHook(const std::string& nick, const std::string& reason) { bot->modUserQuitHook(serverName, nick, reason); }
		void callUserIdentChangeHook(const std::string& nick, const std::string& oldIdent, const std::string& newIdent) { bot->modUserIdentChangeHook(serverName, nick, oldIdent, newIdent); }
		void callUserHostChangeHook(const std::string& nick, const std::string& oldHost, const std::string& newHost) { bot->modUserHostChangeHook(serverName, nick, oldHost, newHost); }
		void callUserGecosChangeHook(const std::string& nick, const std::string& oldGecos, const std::string& newGecos) { bot->modUserGecosChangeHook(serverName, nick, oldGecos, newGecos); }
		void callServerWallopsHook(const std::string& nick, const std::string& message) { bot->modServerWallopsHook(serverName, nick, message); }
		void callServerConnectHook(const std::string& newServer) { bot->modServerConnectHook(serverName, newServer); }
		void callServerDisconnectHook(const std::string& quitServer, const std::string& reason) { bot->modServerDisconnectHook(serverName, quitServer, reason); }
		void callServerCapHook(const std::string& subcmd, const std::string& list) { bot->modServerCapHook(serverName, subcmd, list); }
		void callServerCapabHook(const std::vector<std::string>& capabList) { bot->modServerCapabHook(serverName, capabList); }
		void callServerBurstHook() { bot->modServerBurstHook(serverName); }
		void callServerBurstEndHook() { bot->modServerBurstEndHook(serverName); }
		void callOtherDataHook(const std::string& client, const std::vector<std::string>& parsedLine) { bot->modOtherDataHook(serverName, client, parsedLine); }
		
		void callChanMsgOutHook(const std::string& client, const std::string& channel, char status, const std::string& message) { bot->modChanMsgOutHook(serverName, client, channel, status, message, this); }
		void callChanMsgSendHook(const std::string& client, const std::string& channel, char status, const std::string& message) { bot->modChanMsgSendHook(serverName, client, channel, status, message); }
		void callUserMsgOutHook(const std::string& client, const std::string& nick, const std::string& message) { bot->modUserMsgOutHook(serverName, client, nick, message, this); }
		void callUserMsgSendHook(const std::string& client, const std::string& nick, const std::string& message) { bot->modUserMsgSendHook(serverName, client, nick, message); }
		void callChanNoticeOutHook(const std::string& client, const std::string& channel, char status, const std::string& message) { bot->modChanNoticeOutHook(serverName, client, channel, status, message, this); }
		void callChanNoticeSendHook(const std::string& client, const std::string& channel, char status, const std::string& message) { bot->modChanNoticeSendHook(serverName, client, channel, status, message); }
		void callUserNoticeOutHook(const std::string& client, const std::string& nick, const std::string& message) { bot->modUserNoticeOutHook(serverName, client, nick, message, this); }
		void callUserNoticeSendHook(const std::string& client, const std::string& nick, const std::string& message) { bot->modUserNoticeSendHook(serverName, client, nick, message); }
		void callChanCTCPOutHook(const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params) { bot->modChanCTCPOutHook(serverName, client, channel, status, ctcp, params, this); }
		void callChanCTCPSendHook(const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params) { bot->modChanCTCPSendHook(serverName, client, channel, status, ctcp, params); }
		void callUserCTCPOutHook(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) { bot->modUserCTCPOutHook(serverName, client, nick, ctcp, params, this); }
		void callUserCTCPSendHook(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) { bot->modUserCTCPSendHook(serverName, client, nick, ctcp, params); }
		void callChanCTCPReplyOutHook(const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params) { bot->modChanCTCPReplyOutHook(serverName, client, channel, status, ctcp, params, this); }
		void callChanCTCPReplySendHook(const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params) { bot->modChanCTCPReplySendHook(serverName, client, channel, status, ctcp, params); }
		void callUserCTCPReplyOutHook(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) { bot->modUserCTCPReplyOutHook(serverName, client, nick, ctcp, params, this); }
		void callUserCTCPReplySendHook(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) { bot->modUserCTCPReplySendHook(serverName, client, nick, ctcp, params); }
	private:
		Base* bot;
};

#include "base.h"