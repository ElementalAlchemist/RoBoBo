#ifndef PROTOCOL_H // See the PROTOCOL doc file for more information on utilizing this class.
#define PROTOCOL_H
#include "main.h"
#include "socket.h"

class Base;
class Protocol {
	public:
		Protocol(std::string serverAddress, std::tr1::unordered_map<std::string, std::string> confVars, Base* theBase, unsigned short debug);
		virtual ~Protocol();
		virtual unsigned int apiVersion() = 0;
		virtual void connectServer();
		bool stillConnected();
		bool shouldReset();
		virtual bool isClient();
		std::tr1::unordered_map<std::string, std::string> info();
		virtual std::list<std::pair<std::string, char> > prefixes();
		virtual std::set<char> channelTypes();
		virtual std::vector<std::vector<std::string> > channelModes();
		virtual std::list<std::string> channels();
		virtual std::string channelTopic(std::string channel);
		virtual std::set<std::string> channelUsers(std::string channel);
		virtual std::string userIdent(std::string user);
		virtual std::string userHost(std::string user);
		virtual std::pair<std::string, char> userStatus(std::string channel, std::string user);
		virtual std::string compareStatus(std::set<std::string> statuses);
		
		virtual void sendMsg(std::string client, std::string target, std::string message);
		virtual void sendNotice(std::string client, std::string target, std::string message);
		virtual void setMode(std::string client, std::string target, std::string mode);
		virtual void removeMode(std::string client, std::string target, std::string mode);
		virtual void joinChannel(std::string client, std::string channel, std::string key = "");
		virtual void partChannel(std::string client, std::string channel, std::string reason = "");
		virtual void quitServer(std::string reason = "");
		virtual void kickUser(std::string client, std::string channel, std::string user, std::string reason = "");
		virtual void changeNick(std::string client, std::string newNick);
		virtual void oper(std::string client, std::string username, std::string password);
		
		virtual void killUser(std::string client, std::string user, std::string reason);
		virtual void setXLine(std::string client, char lineType, std::string hostmask, time_t duration, std::string reason);
		virtual void removeXLine(std::string client, char lineType, std::string hostmask);
		virtual std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, time_t> > listXLines(); // unordered_map { line type => unordered_map { hostmask => time to expiry } }
		virtual void sendSNotice(char snomask, std::string text);
		
		virtual void sendOther(std::string rawLine);
		
		virtual std::string addClient(std::string nick, std::string ident, std::string host, std::string gecos);
		virtual void removeClient(std::string client, std::string reason = "");
		virtual std::set<std::string> clients();
		virtual std::tr1::unordered_map<std::string, std::string> clientInfo(std::string client);
		virtual std::list<std::string> userModes(std::string client);
	protected:
		std::string serverName;
		bool keepServer;
		unsigned short debugLevel;
		std::tr1::unordered_map<std::string, std::string> serverConf;
		Socket* connection;
		std::vector<std::string> parseLine(std::string rawLine);
		
		bool callChanMsgHook(std::string client, std::string channel, char target, std::string nick, std::string message);
		bool callUserMsgHook(std::string client, std::string nick, std::string message);
		bool callChanNoticeHook(std::string client, std::string channel, char target, std::string nick, std::string message);
		bool callUserNoticeHook(std::string client, std::string nick, std::string message);
		bool callChannelCTCPHook(std::string client, std::string channel, char target, std::string nick, std::string message);
		bool callUserCTCPHook(std::string client, std::string nick, std::string message);
		bool callChannelCTCPReplyHook(std::string client, std::string channel, char target, std::string nick, std::string message);
		bool callUserCTCPReplyHook(std::string client, std::string nick, std::string message);
		void callChannelJoinPreHook(std::string channel, std::string hostmask);
		void callChannelJoinPostHook(std::string channel, std::string hostmask);
		void callChannelPartPreHook(std::string channel, std::string hostmask, std::string reason);
		void callChannelPartPostHook(std::string channel, std::string hostmask, std::string reason);
		void callUserConnectPreHook(std::string nick, std::string ident, std::string host, std::string gecos);
		void callUserConnectPostHook(std::string nick, std::string ident, std::string host, std::string gecos);
		void callUserQuitPreHook(std::string hostmask, std::string reason);
		void callUserQuitPostHook(std::string hostmask, std::string reason);
		void callNickChangePreHook(std::string oldNick, std::string newNick);
		void callNickChangePostHook(std::string oldNick, std::string newNick);
		void callChannelKickPreHook(std::string channel, std::string kicker, std::string kickee, std::string reason);
		void callChannelKickPostHook(std::string channel, std::string kicker, std::string kickee, std::string reason);
		void callChannelModePreHook(std::string channel, std::string setter, std::string mode, bool add, std::string param = "");
		void callChannelModePostHook(std::string channel, std::string setter, std::string mode, bool add, std::string param = "");
		void callUserModePreHook(std::string client, std::string mode, bool add);
		void callUserModePostHook(std::string client, std::string mode, bool add);
		void callUserOperPreHook(std::string user, std::string opertype);
		void callUserOperPostHook(std::string user, std::string opertype);
		void callNumericHook(std::string client, std::string numeric, std::vector<std::string> parsedLine);
		void callOtherDataHook(std::string client, std::vector<std::string> parsedLine);
		void callPreConnectHook();
		void callConnectHook(std::string client);
		void callQuitHook(std::string client);
		
		std::string callChannelMessageOutHook(std::string client, std::string target, char status, std::string message);
		void callChannelMessageSendHook(std::string client, std::string target, char status, std::string message);
		std::string callUserMessageOutHook(std::string client, std::string target, std::string message);
		void callUserMessageSendHook(std::string client, std::string target, std::string message);
		std::string callChannelNoticeOutHook(std::string client, std::string target, char status, std::string message);
		void callChannelNoticeSendHook(std::string client, std::string target, char status, std::string message);
		std::string callUserNoticeOutHook(std::string client, std::string target, std::string message);
		void callUserNoticeSendHook(std::string client, std::string target, std::string message);
		std::string callChannelCTCPOutHook(std::string client, std::string target, char status, std::string message);
		void callChannelCTCPSendHook(std::string client, std::string target, char status, std::string message);
		std::string callUserCTCPOutHook(std::string client, std::string target, std::string message);
		void callUserCTCPSendHook(std::string client, std::string target, std::string message);
		std::string callChannelCTCPReplyOutHook(std::string client, std::string target, char status, std::string message);
		void callChannelCTCPReplySendHook(std::string client, std::string target, char status, std::string message);
		std::string callUserCTCPReplyOutHook(std::string client, std::string target, std::string message);
		void callUserCTCPReplySendHook(std::string client, std::string target, std::string message);
	private:
		Base* botBase;
};
#include "base.h"
#endif