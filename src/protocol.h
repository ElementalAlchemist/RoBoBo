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
		
		virtual void sendPrivMsg(std::string client, std::string target, std::string message);
		virtual void sendNotice(std::string client, std::string target, std::string message);
		virtual void sendCTCP(std::string client, std::string target, std::string ctcp, std::string params);
		virtual void sendCTCPReply(std::string client, std::string target, std::string ctcp, std::string params);
		virtual void setMode(std::string client, std::string target, std::list<std::string> setModes, std::list<std::string> delModes);
		virtual void setSNOMask(std::string client, char snomask, bool add);
		virtual void setChanTopic(std::string client, std::string channel, std::string topic);
		virtual void joinChannel(std::string client, std::string channel, std::string key = "");
		virtual void partChannel(std::string client, std::string channel, std::string reason);
		virtual void kickUser(std::string client, std::string channel, std::string nick, std::string reason);
		virtual void changeNick(std::string client, std::string newNick);
		virtual std::string addClient(std::string nick, std::string ident, std::string host, std::string gecos);
		virtual void removeClient(std::string client);
		virtual void oper(std::string client, std::string username, std::string password);
		virtual void sendSNotice(char snomask, std::string message);
		virtual void setMetadata(std::string target, std::string key, std::string value);
		virtual void setXLine(std::string client, std::string linetype, std::string mask, time_t duration, std::string reason);
		virtual void delXLine(std::string client, std::string linetype, std::string mask);
		virtual void sendOtherData(std::string client, std::string line);
		
		virtual void processedChanMsg(std::string client, std::string target, char status, std::string message);
		virtual void processedUserMsg(std::string client, std::string target, std::string message);
		virtual void processedChanNotice(std::string client, std::string target, char status, std::string message);
		virtual void processedUserNotice(std::string client, std::string target, std::string message);
		virtual void processedChanCTCP(std::string client, std::string target, char status, std::string ctcp, std::string params);
		virtual void processedUserCTCP(std::string client, std::string target, std::string ctcp, std::string params);
		virtual void processedChanCTCPReply(std::string client, std::string target, char status, std::string ctcp, std::string params);
		virtual void processedUserCTCPReply(std::string client, std::string target, std::string ctcp, std::string params);
		
		virtual std::set<std::string> listModes();
		virtual std::set<std::string> paramModes();
		virtual std::set<std::string> modes();
		virtual std::list<std::pair<std::string, char>> statuses();
		virtual std::set<char> channelTypes();
		virtual std::list<std::string> channels();
		virtual std::list<std::string> inChannels(std::string client);
		virtual std::list<std::string> channelUsers(std::string channel);
		virtual bool userInChannel(std::string channel, std::string user);
		virtual std::string channelTopic(std::string channel);
		virtual std::list<std::string> channelModes(std::string channel);
		virtual bool channelHasMode(std::string channel, std::string mode);
		virtual std::string modeParam(std::string channel, std::string mode);
		virtual std::list<std::string> channelListMode(std::string channel, std::string mode);
		virtual bool channelListHasEntry(std::string channel, std::string listMode, std::string entry);
		virtual std::pair<std::string, char> userStatus(std::string channel, std::string user);
		virtual std::pair<std::string, char> compareStatus(std::string status0, std::string status1);
		virtual std::pair<std::string, char> compareStatus(std::string status0, char status1);
		virtual std::pair<std::string, char> compareStatus(char status0, std::string status1);
		virtual std::pair<std::string, char> compareStatus(char status0, char status1);
		virtual bool userHasStatus(std::string channel, std::string user, std::string status);
		virtual bool userHasStatus(std::string channel, std::string user, char status);
		virtual bool userHasStatusOrGreater(std::string channel, std::string user, std::string status);
		virtual bool userHasStatusOrGreater(std::string channel, std::string user, char status);
		virtual std::list<std::string> clients();
		virtual std::list<std::string> userModes(std::string client);
		virtual bool hasUserMode(std::string client, std::string mode);
		virtual std::list<char> snomasks(std::string client);
		virtual bool hasSNOMask(std::string client, char snomask);
		virtual std::list<std::string> userChannels(std::string nick);
	protected:
		std::string serverName, workingDir;
		std::map<std::string, std::string> config;
		bool log;
		unsigned short debugLevel;
		Socket* assignSocket(std::string socketType);
		void killSocket(std::string socketType, Socket* sockptr);
		void callChanMsgHook(std::string client, std::string channel, char status, std::string nick, std::string message);
		void callUserMsgHook(std::string client, std::string nick, std::string message);
		void callChanNoticeHook(std::string client, std::string channel, char status, std::string nick, std::string message);
		void callUserNoticeHook(std::string client, std::string nick, std::string message);
		void callChanCTCPHook(std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string params);
		void callUserCTCPHook(std::string client, std::string nick, std::string ctcp, std::string params);
		void callChanCTCPReplyHook(std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string params);
		void callUserCTCPReplyHook(std::string client, std::string nick, std::string ctcp, std::string params);
		void callChanModeHook(std::string channel, std::string setter, bool add, std::string mode, std::string param);
		void callUserModeHook(std::string nick, bool add, std::string mode);
		void callUserSNOMaskHook(std::string nick, bool add, char snomask);
		void callChanTopicHook(std::string channel, std::string setter, std::string topic);
		void callChanJoinHook(std::string channel, std::string nick);
		void callChanPartHook(std::string channel, std::string nick, std::string reason);
		void callChanKickHook(std::string channel, std::string kicker, std::string kickee, std::string reason);
		void callUserNickHook(std::string oldNick, std::string newNick);
		void callUserConnectHook(std::string nick);
		void callUserQuitHook(std::string nick, std::string reason);
		void callNumericHook(std::string client, std::string numeric, std::vector<std::string> data);
		void callOperHook(std::string nick, std::string operType);
		void callSNoticeHook(char snotype, std::string message);
		void callMetadataHook(std::string target, std::string dataKey, std::string dataValue);
		void callXLineAddHook(std::string lineType, std::string mask, std::string setter, time_t expiry, std::string reason);
		void callXLineRemoveHook(std::string lineType, std::string mask);
		void callServerConnectHook(std::string newServerName);
		void callServerQuitHook(std::string quitServerName, std::string reason);
		void callOtherDataHook(std::string client, std::vector<std::string> lineTokens);
		void callChanMsgOutHook(std::string client, std::string channel, char status, std::string message);
		void callChanMsgSendHook(std::string client, std::string channel, char status, std::string message);
		void callUserMsgOutHook(std::string client, std::string nick, std::string message);
		void callUserMsgSendHook(std::string client, std::string nick, std::string message);
		void callChanNoticeOutHook(std::string client, std::string channel, char status, std::string message);
		void callChanNoticeSendHook(std::string client, std::string channel, char status, std::string message);
		void callUserNoticeSendHook(std::string client, std::string nick, std::string message);
		void callChanCTCPOutHook(std::string client, std::string channel, char status, std::string ctcp, std::string params);
		void callChanCTCPSendHook(std::string client, std::string channel, char status, std::string ctcp, std::string params);
		void callUserCTCPOutHook(std::string client, std::string nick, std::string ctcp, std::string params);
		void callUserCTCPSendHook(std::string client, std::string nick, std::string ctcp, std::string params);
		void callChanCTCPReplyOutHook(std::string client, std::string channel, char status, std::string ctcp, std::string params);
		void callChanCTCPReplySendHook(std::string client, std::string channel, char status, std::string ctcp, std::string params);
		void callUserCTCPReplyOutHook(std::string client, std::string nick, std::string ctcp, std::string params);
		void callUserCTCPReplySendHook(std::string client, std::string nick, std::string ctcp, std::string params);
	private:
		Base* bot;
};

#include "base.h"