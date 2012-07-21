#pragma once
#include "main.h"

#include "socket.h"

class Base; // This needs to be forward-declared for the Base* pointer in Module; the Base class is included below.
enum Priority { PRI_HIGH, PRI_MEDIUM_HIGH, PRI_NORMAL, PRI_MEDIUM_LOW, PRI_LOW };
enum ModLoadResult { MOD_SUCCESS, MOD_ALREADYLOADED, MOD_OPEN_ERROR, MOD_OUT_OF_MEMORY, MOD_INCOMPATIBLE, MOD_NODEPENDS, MOD_FAILURE };

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
		virtual void onChanMode(const std::string& server, const std::string& channel, bool add, const std::string& mode) {}
		virtual void onUserMode(const std::string& server, const std::string& nick, bool add, const std::string& mode) {}
		virtual void onUserSNOmask(const std::string& server, const std::string& nick, bool add, char snomask) {}
		virtual void onChanJoin(const std::string& server, const std::string& channel, const std::string& user) {}
		virtual void onChanPart(const std::string& server, const std::string& channel, const std::string& user, const std::string& reason) {}
		virtual void onChanKick(const std::string& server, const std::string& channel, const std::string& kicker, const std::string& kickee, const std::string& reason) {}
		virtual void onChanTopic(const std::string& server, const std::string& channel, const std::string& nick, const std::string& topic) {}
		virtual void onChanInvite(const std::string& server, const std::string& channel, const std::string& inviter, const std::string& invitee) {}
		virtual void onChanKnock(const std::string& server, const std::string& channel, const std::string& nick, const std::string& reason) {}
		virtual void onUserNick(const std::string& server, const std::string& oldNick, const std::string& newNick) {}
		virtual void onServerPing(const std::string& server, const std::string& sourceServer) {}
		virtual void onServerPong(const std::string& server, const std::string& sourceServer) {}
		virtual void onNumeric(const std::string& server, const std::string& numeric, const std::vector<std::string>& data) {}
		virtual void onUserOper(const std::string& server, const std::string& nick, const std::string& operType) {}
		virtual void onServerNotice(const std::string& server, char snomask, const std::string& message) {}
		virtual void onMetadata(const std::string& server, const std::string& target, const std::string& key, const std::string& value) {}
		virtual void onXLineAdd(const std::string& server, const std::string& lineType, const std::string& mask) {}
		virtual void onXLineRemove(const std::string& server, const std::string& lineType, const std::string& mask) {}
		virtual void onUserConnect(const std::string& server, const std::string& nick) {}
		virtual void onUserQuit(const std::string& server, const std::string& nick, const std::string& reason) {}
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
		virtual void onOtherData(const std::string& server, const std::string& client, const std::vector<std::string>& parsedLine) {}
		
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
		
		void sendPrivMsg(const std::string& server, const std::string& client, const std::string& target, const std::string& message);
		void sendNotice(const std::string& server, const std::string& client, const std::string& target, const std::string& message);
		void sendCTCP(const std::string& server, const std::string& client, const std::string& target, const std::string& ctcp, const std::string& params = "");
		void sendCTCPReply(const std::string& server, const std::string& client, const std::string& target, const std::string& ctcp, const std::string& params = "");
		void setMode(const std::string& server, const std::string& client, const std::string& target, const std::list<std::string>& setModes, const std::list<std::string>& remModes);
		void setSNOmask(const std::string& server, const std::string& client, bool add, char snomask);
		void joinChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& key = "");
		void partChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason);
		void kickUser(const std::string& server, const std::string& client, const std::string& channel, const std::string& user, const std::string& reason);
		std::string addClient(const std::string& server, std::string& nick, std::string& ident, std::string& host, std::string& gecos);
		void removeClient(const std::string& server, const std::string& client);
		void setTopic(const std::string& server, const std::string& client, const std::string& channel, const std::string& topic);
		void inviteUser(const std::string& server, const std::string& client, const std::string& channel, const std::string& user);
		void knockOnChannel(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason);
		void changeNick(const std::string& server, const std::string& user, const std::string& newNick);
		void sendPing(const std::string& server, const std::string& remoteServer);
		void operUp(const std::string& server, const std::string& client, const std::string& usernameOrType, const std::string& password);
		void sendServerNotice(const std::string& server, char snomask, const std::string& message);
		void setMetadata(const std::string& server, const std::string& target, const std::string& key, const std::string& value);
		void setXLine(const std::string& server, const std::string& client, const std::string& lineType, const std::string& mask, time_t duration, const std::string& reason);
		void remXLine(const std::string& server, const std::string& client, const std::string& lineType, const std::string& mask);
		void changeIdent(const std::string& server, const std::string& user, const std::string& newIdent);
		void changeHost(const std::string& server, const std::string& user, const std::string& newHost);
		void changeGecos(const std::string& server, const std::string& user, const std::string& newGecos);
		void sendWallops(const std::string& server, const std::string& client, const std::string& message);
		void sendOtherData(const std::string& server, const std::string& client, const std::string& line);
		
		void connectServer(const std::string& server);
		void disconnectServer(const std::string& server);
		ModLoadResult loadModule(const std::string& modName);
		void unloadModule(const std::string& modName);
		std::shared_ptr<Socket> assignSocket(const std::string& socketType);
		void rehash();
		
		unsigned int botVersion();
		std::list<std::string> activeServers();
		std::list<std::string> connectedServers();
		std::list<std::string> loadedModules();
		std::string serverType(const std::string& server);
		bool serverIsClient(const std::string& server);
		std::set<std::string> providedServices();
		bool serviceIsProvided(const std::string& service);
		std::list<std::string> serviceProviders(const std::string& service);
		std::list<std::string> serviceUsers(const std::string& service);
		void refreshServices();
		
		std::list<std::string> networkServerList(const std::string& server);
		std::list<std::string> xLineTypes(const std::string& server);
		std::list<std::string> xLineList(const std::string& server, const std::string& lineType);
		time_t xLineExpiry(const std::string& server, const std::string& lineType, const std::string& mask);
		std::string xLineReason(const std::string& server, const std::string& lineType, const std::string& mask);
		std::list<std::string> chanListModes(const std::string& server);
		std::list<std::string> chanParamModes(const std::string& server);
		std::list<std::string> chanNoParamModes(const std::string& server);
		std::list<std::pair<std::string, char>> chanPrefixes(const std::string& server);
		std::pair<std::string, char> compareStatus(const std::string& server, const std::string& status0, const std::string& status1);
		std::pair<std::string, char> compareStatus(const std::string& server, const std::string& status0, char status1);
		std::pair<std::string, char> compareStatus(const std::string& server, char status0, const std::string& status1);
		std::pair<std::string, char> compareStatus(const std::string& server, char status0, char status1);
		
		std::string chanTopic(const std::string& server, const std::string& channel);
		time_t chanTimestamp(const std::string& server, const std::string& channel);
		std::set<std::string> chanUsers(const std::string& server, const std::string& channel);
		bool userInChan(const std::string& server, const std::string& channel, const std::string& user);
		std::pair<std::string, char> userStatus(const std::string& server, const std::string& channel, const std::string& user);
		bool userHasStatus(const std::string& server, const std::string& channel, const std::string& user, const std::string& status);
		bool userHasStatus(const std::string& server, const std::string& channel, const std::string& user, char status);
		bool userHasStatusOrGreater(const std::string& server, const std::string& channel, const std::string& user, const std::string& status);
		bool userHasStatusOrGreater(const std::string& server, const std::string& channel, const std::string& user, char status);
		std::list<std::string> chanModes(const std::string& server, const std::string& channel);
		std::list<std::string> chanListModeList(const std::string& server, const std::string& channel, const std::string& listMode);
		bool chanHasMode(const std::string& server, const std::string& channel, const std::string& mode);
		std::string chanModeParam(const std::string& server, const std::string& channel, const std::string& mode);
		
		std::list<std::string> clientList(const std::string& server);
		std::string clientNick(const std::string& server, const std::string& client);
		std::string userIdent(const std::string& server, const std::string& user);
		std::string userHost(const std::string& server, const std::string& user);
		std::string userGecos(const std::string& server, const std::string& user);
		std::set<std::string> userModes(const std::string& server, const std::string& user);
		bool userHasMode(const std::string& server, const std::string& user, const std::string& mode);
		std::set<char> userSNOmasks(const std::string& server, const std::string& user);
		bool userHasSNOmask(const std::string& server, const std::string& user, char snomask);
		std::set<std::string> userChans(const std::string& server, const std::string& user);
		time_t userTimestamp(const std::string& server, const std::string& user);
		time_t userNickTimestamp(const std::string& server, const std::string& user);
	private:
		Base* bot;
};

#include "base.h"