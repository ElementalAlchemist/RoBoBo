#pragma once
#include "main.h"

enum Priority { PRI_HIGH, PRI_MEDIUM_HIGH, PRI_NORMAL, PRI_MEDIUM_LOW, PRI_LOW };

#define MODULE_SPAWN(modName) extern "C" Module* spawn(std::string moduleName, std::map<std::string, std::string> config, std::string workingDir, unsigned short debugLevel, Base* botptr) {\
		return new modName (moduleName, config, workingDir, debugLevel, botptr);\
	}

typedef bool MsgAction;
const bool MSG_CONTINUE = true;
const bool MSG_IGNORE = false;

class Module {
	public:
		Module(std::string modName, std::map<std::string, std::string> conf, std::string workDir, unsigned short debug, Base* botptr);
		const Priority priority;
		virtual unsigned int apiVersion() = 0;
		virtual bool onLoadComplete();
		virtual void onRehash();
		virtual void onModuleLoad(std::string modName);
		virtual void onModuleUnload(std::string modName);
		void rehash(std::map<std::string, std::string> conf);
		void endDebug();
		
		virtual MsgAction onChanMsg(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message);
		virtual MsgAction onUserMsg(std::string server, std::string client, std::string nick, std::string message);
		virtual MsgAction onChanNotice(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message);
		virtual MsgAction onUserNotice(std::string server, std::string client, std::string nick, std::string message);
		virtual MsgAction onChanCTCP(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string data);
		virtual MsgAction onUserCTCP(std::string server, std::string client, std::string nick, std::string ctcp, std::string data);
		virtual MsgAction onChanCTCPReply(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string data);
		virtual MsgAction onUserCTCPReply(std::string server, std::string client, std::string nick, std::string ctcp, std::string data);
		virtual void onChanMode(std::string server, std::string client, std::string channel, bool add, std::string mode, std::string param);
		virtual void onUserMode(std::string server, std::string client, bool add, std::string mode);
		virtual void onUserSNOMask(std::string server, std::string client, bool add, char snomask);
		virtual void onChanTopic(std::string server, std::string client, std::string channel, std::string topic);
		virtual void onChanJoin(std::string server, std::string client, std::string channel, std::string nick);
		virtual void onChanPart(std::string server, std::string client, std::string channel, std::string nick, std::string reason);
		virtual void onUserConnect(std::string server, std::string client, std::string nick);
		virtual void onUserQuit(std::string server, std::string client, std::string nick, std::string reason);
		virtual void onNumeric(std::string server, std::string client, std::string numeric, std::vector<std::string> data);
		virtual void onOper(std::string server, std::string nick, std::string operType);
		virtual void onSNotice(std::string server, std::string snotype, std::string message);
		virtual void onUserMetadata(std::string server, std::string nick, std::string dataKey, std::string dataValue);
		virtual void onServerData(std::string server, std::string dataType, std::string params);
		virtual void onXLineAdd(std::string server, std::string lineType, std::string mask, std::string setter, time_t expiry, std::string reason);
		virtual void onXLineRemove(std::string server, std::string lineType, std::string mask);
		virtual void onOtherData(std::string server, std::string client, std::vector<std::string> lineTokens);
		
		virtual void onChanMsgOut(std::string server, std::string client, std::string channel, char status, std::string &message);
		virtual void onChanMsgSend(std::string server, std::string client, std::string channel, char status, std::string message);
		virtual void onUserMsgOut(std::string server, std::string client, std::string nick, std::string &message);
		virtual void onUserMsgSend(std::string server, std::string client, std::string nick, std::string message);
		virtual void onChanNoticeOut(std::string server, std::string client, std::string channel, char status, std::string &message);
		virtual void onChanNoticeSend(std::string server, std::string client, std::string channel, char status, std::string message);
		virtual void onUserNoticeOut(std::string server, std::string client, std::string nick, std::string &message);
		virtual void onUserNoticeSend(std::string server, std::string client, std::string nick, std::string message);
		virtual void onChanCTCPOut(std::string server, std::string client, std::string channel, char status, std::string &ctcp, std::string &params);
		virtual void onChanCTCPSend(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params);
		virtual void onUserCTCPOut(std::string server, std::string client, std::string nick, std::string &ctcp, std::string &params);
		virtual void onUserCTCPSend(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
		virtual void onChanCTCPReplyOut(std::string server, std::string client, std::string channel, char status, std::string &ctcp, std::string &params);
		virtual void onChanCTCPReplySend(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params);
		virtual void onUserCTCPReplyOut(std::string server, std::string client, std::string nick, std::string &ctcp, std::string &params);
		virtual void onUserCTCPReplySend(std::string server, std::string client, std::string nick, std::string ctcp, std::string params);
		
		virtual std::string description();
		virtual std::list<std::string> provides();
		virtual std::list<std::string> requires();
		virtual std::list<std::string> supports();
	protected:
		std::string moduleName;
		std::map<std::string, std::string> config;
		std::string workingDir;
		unsigned short debugLevel;
	private:
		Base* bot;
};