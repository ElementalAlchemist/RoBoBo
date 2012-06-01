#pragma once
#include "main.h"

#include "base.h"

class Protocol {
	public:
		Protocol(std::string server, std::map<std::string, std::string> conf, std::string workDir, bool dumpLogs, unsigned short debug, Base* botptr);
		virtual ~Protocol();
		virtual unsigned int apiVersion() = 0;
		virtual void connectServer();
		virtual void disconnectServer();
		virtual bool isConnected();
		bool shouldReset();
		
		virtual void sendPrivMsg(std::string client, std::string target, std::string message);
		virtual void sendNotice(std::string client, std::string target, std::string message);
		virtual void sendCTCP(std::string client, std::string target, std::string ctcp);
		virtual void sendCTCPReply(std::string client, std::string target, std::string ctcp);
		virtual void setMode(std::string client, std::string target, std::list<std::string> setModes, std::list<std::string> delModes);
		virtual void setSNOMask(std::string client, std::string snomask, bool add);
		virtual void setChanTopic(std::string client, std::string channel, std::string topic);
		virtual void joinChannel(std::string client, std::string channel, std::string key = "");
		virtual void partChannel(std::string client, std::string channel, std::string reason);
		virtual std::string addClient(std::string nick, std::string ident, std::string host, std::string gecos);
		virtual void removeClient(std::string client);
		virtual void changeNick(std::string client, std::string newNick);
		virtual void oper(std::string client, std::string username, std::string password);
		virtual void sendSNotice(std::string snomask, std::string message);
		virtual void setMetadata(std::string target, std::string key, std::string value);
		virtual void setXLine(std::string client, std::string linetype, std::string mask, time_t duration, std::string reason);
		virtual void delXLine(std::string client, std::string linetype, std::string mask);
		virtual void sendOtherData(std::string client, std::string line);
		
		virtual void processedChanMsg(std::string client, std::string target, char status, std::string message);
		virtual void processedUserMsg(std::string client, std::string target, std::string message);
		virtual void processedChanNotice(std::string client, std::string target, char status, std::string message);
		virtual void processedUserNotice(std::string client, std::string target, std::string message);
		virtual void processedChanCTCP(std::string client, std::string target, char status, std::string ctcp);
		virtual void processedUserCTCP(std::string client, std::string target, std::string ctcp);
		virtual void processedChanCTCPReply(std::string client, std::string target, char status, std::string ctcp);
		virtual void processedUserCTCPReply(std::string client, std::string target, std::string ctcp);
	protected:
		std::string serverName, workingDir;
		std::map<std::string, std::string> config;
		bool log;
		unsigned short debugLevel;
		Socket* assignSocket(std::string socketType);
		void killSocket(std::string socketType, Socket* sockptr);
		bool resetConnection;
		
		void callChanMsgHook(std::string client, std::string channel, char status, std::string nick, std::string message);
		void callUserMsgHook(std::string client, std::string nick, std::string message);
		void callChanNoticeHook(std::string client, std::string channel, char status, std::string nick, std::string message);
		void callUserNoticeHook(std::string client, std::string nick, std::string message);
		void callChanCTCPHook(std::string client, std::string channel, char status, std::string nick, std::string ctcp);
		void callUserCTCPHook(std::string client, std::string nick, std::string ctcp);
		void callChanCTCPReplyHook(std::string client, std::string channel, char status, std::string nick, std::string ctcp);
		void callUserCTCPReplyHook(std::string client, std::string nick, std::string ctcp);
		void callChanModeHook(std::string client, std::string channel, bool add, std::string mode, std::string param);
		void callUserModeHook(std::string client, bool add, std::string mode);
		void callUserSNOMaskHook(std::string client, bool add, std::string snomask);
		void callChanTopicHook(std::string client, std::string channel, std::string topic);
		void callChanJoinHook(std::string client, std::string channel, std::string nick);
		void callChanPartHook(std::string client, std::string channel, std::string nick, std::string reason);
		void callUserConnectHook(std::string nick);
		void callUserQuitHook(std::string client, std::string nick, std::string reason);
		void callUserNickHook(std::string client, std::string oldNick, std::string newNick);
		void callNumericHook(std::string client, std::string numeric, std::vector<std::string> data);
		void callOperHook(std::string nick, std::string operType);
		void callSNoticeHook(std::string snotype, std::string message);
		void callMetadataHook(std::string target, std::string dataKey, std::string dataValue);
		void callServerDataHook(std::string dataType, std::vector<std::string> params);
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
		void callUserNoticeOutHook(std::string client, std::string nick, std::string message);
		void callUserNoticeSendHook(std::string client, std::string nick, std::string message);
		void callChanCTCPOutHook(std::string client, std::string channel, char status, std::string ctcp);
		void callChanCTCPSendHook(std::string client, std::string channel, char status, std::string ctcp);
		void callUserCTCPOutHook(std::string client, std::string nick, std::string ctcp);
		void callUserCTCPSendHook(std::string client, std::string nick, std::string ctcp);
		void callChanCTCPReplyOutHook(std::string client, std::string channel, char status, std::string ctcp);
		void callChanCTCPReplySendHook(std::string client, std::string channel, char status, std::string ctcp);
		void callUserCTCPReplyOutHook(std::string client, std::string nick, std::string ctcp);
		void callUserCTCPReplySendHook(std::string client, std::string nick, std::string ctcp);
	private:
		Base* bot;
};