#pragma once
#include "main.h"

#include "socket.h"

class Base;

class Protocol {
	public:
		Protocol(std::string server, std::map<std::string, std::string> conf, std::string workDir, bool dumpLogs, unsigned short debug, Base* botptr);
		virtual ~Protocol();
		virtual unsigned int apiVersion() = 0;
		virtual void connectServer() {}
		virtual void disconnectServer(std::string reason) {}
		virtual bool isConnected() { return false; }
		virtual bool deadServer() { return true; }
		virtual bool isClient() { return false; }
		void endDebug() { debugLevel = 0; }
		
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
		virtual std::set<std::string> chanTypes() { return std::set<std::string> (); }
		virtual std::list<std::string> chanListModes() { return std::list<std::string> (); }
		virtual std::list<std::string> chanParamModes() { return std::list<std::string> (); }
		virtual std::list<std::string> chanNoParamModes() { return std::list<std::string> (); }
		virtual std::list<std::pair<std::string, char>> chanPrefixes() { return std::list<std::pair<std::string, char>> (); }
		virtual std::pair<std::string, char> compareStatus(const std::string& status0, const std::string& status1) { return std::pair<std::string, char> ("", ' '); }
		virtual std::pair<std::string, char> compareStatus(const std::string& status0, char status1) { return std::pair<std::string, char> ("", ' '); }
		virtual std::pair<std::string, char> compareStatus(char status0, char status1) { return std::pair<std::string, char> ("", ' '); }
		
		virtual std::list<std::string> chanList() { return std::list<std::string> (); }
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
		virtual std::set<std::string> userModes(const std::string& user) { return std::set<std::string> (); }
		virtual bool userHasMode(const std::string& user, const std::string& mode) { return false; }
		virtual std::set<char> userSNOmasks(const std::string& user) { return std::set<char> (); }
		virtual bool userHasSNOmask(const std::string& user, char snomask) { return false; }
		virtual std::set<std::string> userChans(const std::string& user) { return std::set<std::string> (); }
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
		
		void callChanMsgHook(const std::string& client, const std::string& channel, char status, const std::string& nick, const std::string& message);
		void callUserMsgHook(const std::string& client, const std::string& nick, const std::string& message);
		void callChanNoticeHook(const std::string& client, const std::string& channel, char status, const std::string& nick, const std::string& message);
		void callUserNoticeHook(const std::string& client, const std::string& nick, const std::string& message);
		void callChanCTCPHook(const std::string& client, const std::string& channel, char status, const std::string& nick, const std::string& ctcp, const std::string& params);
		void callUserCTCPHook(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params);
		void callChanCTCPReplyHook(const std::string& client, const std::string& channel, char status, const std::string& nick, const std::string& ctcp, const std::string& params);
		void callUserCTCPReplyHook(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params);
		void callChanModeHook(const std::string& channel, bool add, const std::string& mode);
		void callUserModeHook(const std::string& nick, bool add, const std::string& mode);
		void callUserSNOmaskHook(const std::string& nick, bool add, char snomask);
		void callChanJoinHook(const std::string& channel, const std::string& user);
		void callChanPartHook(const std::string& channel, const std::string& user, const std::string& reason);
		void callChanKickHook(const std::string& channel, const std::string& kicker, const std::string& kickee, const std::string& reason);
		void callChanTopicHook(const std::string& channel, const std::string& nick, const std::string& topic);
		void callChanInviteHook(const std::string& channel, const std::string& inviter, const std::string& invitee);
		void callChanKnockHook(const std::string& channel, const std::string& nick, const std::string& reason);
		void callUserNickHook(const std::string& oldNick, const std::string& newNick);
		void callServerPingHook(const std::string& sourceServer);
		void callServerPongHook(const std::string& sourceServer);
		void callNumericHook(const std::string& client, const std::string& numeric, const std::vector<std::string>& data);
		void callUserOperHook(const std::string& nick, const std::string& operType);
		void callServerNoticeHook(char snomask, const std::string& message);
		void callMetadataHook(const std::string& target, const std::string& key, const std::string& value);
		void callXLineAddHook(const std::string& lineType, const std::string& mask);
		void callXLineRemoveHook(const std::string& lineType, const std::string& mask);
		void callUserConnectHook(const std::string& nick);
		void callUserQuitHook(const std::string& nick, const std::string& reason);
		void callUserIdentChangeHook(const std::string& nick, const std::string& oldIdent, const std::string& newIdent);
		void callUserHostChangeHook(const std::string& nick, const std::string& oldHost, const std::string& newHost);
		void callUserGecosChangeHook(const std::string& nick, const std::string& oldGecos, const std::string& newGecos);
		void callServerWallopsHook(const std::string& nick, const std::string& message);
		void callServerConnectHook(const std::string& newServer);
		void callServerDisconnectHook(const std::string& quitServer, const std::string& reason);
		void callServerCapHook(const std::string& client, const std::string& subcmd, const std::string& list);
		void callServerCapabHook(const std::vector<std::string>& capabList);
		void callServerBurstHook();
		void callServerBurstEndHook();
		void callOtherDataHook(const std::string& client, const std::vector<std::string>& parsedLine);
		
		void callChanMsgOutHook(const std::string& client, const std::string& channel, char status, const std::string& message);
		void callChanMsgSendHook(const std::string& client, const std::string& channel, char status, const std::string& message);
		void callUserMsgOutHook(const std::string& client, const std::string& nick, const std::string& message);
		void callUserMsgSendHook(const std::string& client, const std::string& nick, const std::string& message);
		void callChanNoticeOutHook(const std::string& client, const std::string& channel, char status, const std::string& message);
		void callChanNoticeSendHook(const std::string& client, const std::string& channel, char status, const std::string& message);
		void callUserNoticeOutHook(const std::string& client, const std::string& nick, const std::string& message);
		void callUserNoticeSendHook(const std::string& client, const std::string& nick, const std::string& message);
		void callChanCTCPOutHook(const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params);
		void callChanCTCPSendHook(const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params);
		void callUserCTCPOutHook(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params);
		void callUserCTCPSendHook(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params);
		void callChanCTCPReplyOutHook(const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params);
		void callChanCTCPReplySendHook(const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params);
		void callUserCTCPReplyOutHook(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params);
		void callUserCTCPReplySendHook(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params);
	private:
		Base* bot;
};

#include "base.h"