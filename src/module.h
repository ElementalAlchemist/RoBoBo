#pragma once
#include "main.h"

#include "socket.h"

class Base; // This needs to be forward-declared for the Base* pointer in Module; the Base class is included below.
enum LoadResult { LOAD_SUCCESS, LOAD_ALREADYLOADED, LOAD_OPEN_ERROR, LOAD_INCOMPATIBLE, LOAD_NODEPENDS, LOAD_FAILURE };
enum Priority { PRI_HIGH, PRI_MEDIUM_HIGH, PRI_NORMAL, PRI_MEDIUM_LOW, PRI_LOW };

typedef bool MsgAction;
const bool MSG_CONTINUE = true;
const bool MSG_IGNORE = false;

class Module {
	public:
		Module(const std::string& modName, const std::map<std::string, std::string>& conf, const std::string& workDir, unsigned short debug, Base* botptr);
		virtual ~Module();
		virtual Priority priority() { return PRI_NORMAL; }
		virtual unsigned int apiVersion() = 0;
		virtual bool onLoadComplete() { return true; }
		virtual void onUnload() {}
		virtual void onRehash() {}
		virtual void onModuleLoad(const std::string& modName) {}
		virtual void onModuleUnload(const std::string& modName) {}
		virtual bool forceKeepAlive() { return false; }
		void rehash(const std::map<std::string, std::string>& conf) { config = conf; }
		void endDebug() { debugLevel = 0; }
		
		virtual MsgAction onChanMsg(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& nick, const std::string& message) { return MSG_CONTINUE; }
		virtual MsgAction onUserMsg(const std::string& server, const std::string& client, const std::string& nick, const std::string& message) { return MSG_CONTINUE; }
		virtual MsgAction onChanNotice(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& nick, const std::string& message) { return MSG_CONTINUE; }
		virtual MsgAction onUserNotice(const std::string& server, const std::string& client, const std::string& nick, const std::string& message) { return MSG_CONTINUE; }
		virtual MsgAction onChanCTCP(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& nick, const std::string& ctcp, const std::string& params) { return MSG_CONTINUE; }
		virtual MsgAction onUserCTCP(const std::string& server, const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) { return MSG_CONTINUE; }
		virtual MsgAction onChanCTCPReply(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& nick, const std::string& ctcp, const std::string& params) { return MSG_CONTINUE; }
		virtual MsgAction onUserCTCPReply(const std::string& server, const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) { return MSG_CONTINUE; }
		virtual void onChanMode(const std::string& server, const std::string& client, const std::string& channel, bool add, const std::string& mode) {}
		virtual void onUserMode(const std::string& server, const std::string& nick, bool add, const std::string& mode) {}
		virtual void onUserSNOmask(const std::string& server, const std::string& nick, bool add, char snomask) {}
		virtual void onChanJoin(const std::string& server, const std::string& channel, const std::string& user) {}
		virtual void onChanPart(const std::string& server, const std::string& channel, const std::string& user, const std::string& reason) {}
		virtual void onChanKick(const std::string& server, const std::string& channel, const std::string& kicker, const std::string& kickee, const std::string& reason) {}
		virtual void onUserConnect(const std::string& server, const std::string& nick) {}
		virtual void onUserQuit(const std::string& server, const std::string& nick, const std::string& reason) {}
		virtual void onChanTopic(const std::string& server, const std::string& channel, const std::string& nick, const std::string& topic) {}
		virtual void onChanInvite(const std::string& server, const std::string& channel, const std::string& inviter, const std::string& invitee) {}
		virtual void onChanKnock(const std::string& server, const std::string& channel, const std::string& nick, const std::string& reason) {}
		virtual void onUserNick(const std::string& server, const std::string& oldNick, const std::string& newNick) {}
		virtual void onServerPing(const std::string& server, const std::string& sourceServer) {}
		virtual void onServerPong(const std::string& server, const std::string& sourceServer) {}
		virtual void onNumeric(const std::string& server, const std::string& numeric, const std::string& data) {}
		virtual void onUserOper(const std::string& server, const std::string& nick, const std::string& operType) {}
		virtual void onServerNotice(const std::string& server, char snomask, const std::string& message) {}
		virtual void onMetadata(const std::string& server, const std::string& target, const std::string& key, const std::string& value) {}
		virtual void onXLineAdd(const std::string& server, const std::string& lineType, const std::string& mask) {}
		virtual void onXLineRemove(const std::string& server, const std::string& lineType, const std::string& mask) {}
		virtual void onUserIdentChange(const std::string& server, const std::string& nick, const std::string& oldIdent, const std::string& newIdent) {}
		virtual void onUserHostChange(const std::string& server, const std::string& nick, const std::string& oldHost, const std::string& newHost) {}
		virtual void onUserGecosChange(const std::string& server, const std::string& nick, const std::string& oldGecos, const std::string& newGecos) {}
		virtual void onServerWallops(const std::string& server, const std::string& nick, const std::string& message) {}
		virtual void onServerConnect(const std::string& server, const std::string& serverName) {}
		virtual void onServerDisconnect(const std::string& server, const std::string& serverName, const std::string& reason) {}
		virtual void onServerCap(const std::string& server, const std::string& subcmd, const std::string& list) {}
		virtual void onServerCapab(const std::string& server, const std::vector<std::string>& capabList) {}
		virtual void onServerBurst(const std::string& server) {}
		virtual void onServerBurstEnd(const std::string& server) {}
		virtual void onConnect(const std::string& server) {}
		virtual void onDisconnect(const std::string& server) {}
		
		virtual void onChanMsgOut(const std::string& server, const std::string& client, const std::string& channel, char status, std::string& message) {}
		virtual void onChanMsgSend(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& message) {}
		virtual void onUserMsgOut(const std::string& server, const std::string& client, const std::string& nick, std::string& message) {}
		virtual void onUserMsgSend(const std::string& server, const std::string& client, const std::string& nick, const std::string& message) {}
		virtual void onChanNoticeOut(const std::string& server, const std::string& client, const std::string& channel, char status, std::string& message) {}
		virtual void onChanNoticeSend(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& message) {}
		virtual void onUserNoticeOut(const std::string& server, const std::string& client, const std::string& nick, std::string& message) {}
		virtual void onUserNoticeSend(const std::string& server, const std::string& client, const std::string& nick, const std::string& message) {}
		virtual void onChanCTCPOut(const std::string& server, const std::string& client, const std::string& channel, char status, std::string& ctcp, std::string& params) {}
		virtual void onChanCTCPSend(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params) {}
		virtual void onUserCTCPOut(const std::string& server, const std::string& client, const std::string& nick, std::string& ctcp, std::string& params) {}
		virtual void onUserCTCPSend(const std::string& server, const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) {}
		virtual void onChanCTCPReplyOut(const std::string& server, const std::string& client, const std::string& channel, char status, std::string& ctcp, std::string& params) {}
		virtual void onChanCTCPReplySend(const std::string& server, const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params) {}
		virtual void onUserCTCPReplyOut(const std::string& server, const std::string& client, const std::string& nick, std::string& ctcp, std::string& params) {}
		virtual void onUserCTCPReplySend(const std::string& server, const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) {}
		
		virtual std::string description() { return "A description has not been provided by the module author."; }
		virtual std::list<std::string> provides() { return std::list<std::string> (); }
		virtual std::list<std::string> requires() { return std::list<std::string> (); }
		virtual std::list<std::string> supports() { return std::list<std::string> (); }
	protected:
		const std::string moduleName, workingDir;
		std::map<std::string, std::string> config;
		unsigned short debugLevel;
		
		void sendPrivMsg(const std::string& server, const std::string& client, const std::string& target, const std::string& message) { bot->sendPrivMsg(server, client, target, message); }
		void sendNotice(const std::string& server, const std::string& client, const std::string& target, const std::string& message) { bot->sendNotice(server, client, target, message); }
		void sendCTCP(const std::string& server, const std::string& client, const std::string& target, const std::string& ctcp, const std::string& params = "") { bot->sendCTCP(server, client, target, ctcp, params); }
		void sendCTCPReply(const std::string& server, const std::string& client, const std::string& target, const std::string& ctcp, const std::string& params = "") { bot->sendCTCPReply(server, client, target, ctcp, params); }
		void setMode(const std::string& server, const std::string& client, const std::string& target, const std::list<std::string>& setMode, const std::list<std::string>& remMode) { bot->setMode(server, client, target, setMode, remMode); }
		void joinChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& key = "") { bot->joinChan(server, client, channel, key); }
		void partChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason) { bot->partChan(server, client, channel, reason); }
		void kickUser(const std::string& server, const std::string& client, const std::string& channel, const std::string& user, const std::string& reason) { bot->kickUser(server, client, channel, user, reason); }
		std::string addClient(const std::string& server, std::string& nick, std::string& ident, std::string& host, std::string& gecos) { return bot->addClient(server, nick, ident, host, gecos); }
		void removeClient(const std::string& server, const std::string& client) { bot->removeClient(server, client); }
		void setTopic(const std::string& server, const std::string& client, const std::string& channel, const std::string& topic) { bot->setTopic(server, client, channel, topic); }
		void inviteUser(const std::string& server, const std::string& client, const std::string& channel, const std::string& user) { bot->inviteUser(server, client, channel, user); }
		void knockOnChannel(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason) { bot->knockOnChannel(server, client, channel, reason); }
		void changeNick(const std::string& server, const std::string& user, const std::string& newNick) { bot->changeNick(server, user, newNick); }
		void sendPing(const std::string& server, const std::string& remoteServer) { bot->sendPing(server, remoteServer); }
		void operUp(const std::string& server, const std::string& client, const std::string& usernameOrType, const std::string& password) { bot->operUp(server, client, usernameOrType, password); }
		void sendServerNotice(const std::string& server, char snomask, const std::string& message) { bot->sendServerNotice(server, snomask, message); }
		void setMetadata(const std::string& server, const std::string& target, const std::string& key, const std::string& value) { bot->setMetadata(server, target, key, value); }
		void setXLine(const std::string& server, const std::string& client, const std::string& lineType, const std::string& mask, time_t duration, const std::string& reason) { bot->setXLine(server, client, lineType, mask, duration, reason); }
		void remXLine(const std::string& server, const std::string& client, const std::stirng& lineType, const std::string& mask) { bot->remXLine(server, client, lineType, mask); }
		void changeIdent(const std::string& server, const std::string& user, const std::string& newIdent) { bot->changeIdent(server, user, newIdent); }
		void changeHost(const std::string& server, const std::string& user, const std::string& newHost) { bot->changeHost(server, user, newHost); }
		void changeGecos(const std::string& server, const std::stirng& user, const std::string& newGecos) { bot->changeGecos(server, user, newGecos); }
		void sendWallops(const std::string& server, const std::string& client, const std::string& message) { bot->sendWallops(server, client, message); }
		
		void connectServer(const std::string& server) { bot->connectServer(server); }
		void disconnectServer(const std::string& server) { bot->disconnectServer(server); }
		LoadResult loadModule(const std::string& modName) { return bot->loadModule(modName); }
		void unloadModule(const std::string& modName) { if (modName == moduleName) std::thread(&Base::unloadModule, bot, modName, true).detach(); else bot->unloadModule(modName, true); }
		SocketHandle assignSocket(const std::string& socketType) { return bot->assignSocket(socketType); }
		void rehash() { bot->rehash(); }
	private:
		Base* bot;
};

#include "base.h"