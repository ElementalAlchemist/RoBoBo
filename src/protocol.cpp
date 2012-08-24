#include "protocol.h"

Protocol::Protocol(std::string server, std::map<std::string, std::string> conf, std::string workDir, bool dumpLogs, unsigned short debug, Base* botptr) : serverName(server), workingDir(workDir), config(conf), log(dumpLogs), debugLevel(debug), bot(botptr) {}

Protocol::~Protocol() {}

inline std::shared_ptr<Socket> Protocol::assignSocket(std::string socketType) {
	return bot->loadSocket(socketType);
}

inline void Protocol::callChanMsgHook(const std::string& client, const std::string& channel, char status, const std::string& nick, const std::string& message) {
	bot->modChanMsgHook(serverName, client, channel, status, nick, message);
}

inline void Protocol::callUserMsgHook(const std::string& client, const std::string& nick, const std::string& message) {
	bot->modUserMsgHook(serverName, client, nick, message);
}

inline void Protocol::callChanNoticeHook(const std::string& client, const std::string& channel, char status, const std::string& nick, const std::string& message) {
	bot->modChanNoticeHook(serverName, client, channel, status, nick, message);
}

inline void Protocol::callUserNoticeHook(const std::string& client, const std::string& nick, const std::string& message) {
	bot->modUserNoticeHook(serverName, client, nick, message);
}

inline void Protocol::callChanCTCPHook(const std::string& client, const std::string& channel, char status, const std::string& nick, const std::string& ctcp, const std::string& params) {
	bot->modChanCTCPHook(serverName, client, channel, status, nick, ctcp, params);
}

inline void Protocol::callUserCTCPHook(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) {
	bot->modUserCTCPHook(serverName, client, nick, ctcp, params);
}

inline void Protocol::callChanCTCPReplyHook(const std::string& client, const std::string& channel, char status, const std::string& nick, const std::string& ctcp, const std::string& params) {
	bot->modChanCTCPReplyHook(serverName, client, channel, status, nick, ctcp, params);
}

inline void Protocol::callUserCTCPReplyHook(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) {
	bot->modUserCTCPReplyHook(serverName, client, nick, ctcp, params);
}

inline void Protocol::callChanModeHook(const std::string& channel, bool add, const std::string& mode) {
	bot->modChanModeHook(serverName, channel, add, mode);
}

inline void Protocol::callUserModeHook(const std::string& nick, bool add, const std::string& mode) {
	bot->modUserModeHook(serverName, nick, add, mode);
}

inline void Protocol::callUserSNOmaskHook(const std::string& nick, bool add, char snomask) {
	bot->modUserSNOmaskHook(serverName, nick, add, snomask);
}

inline void Protocol::callChanJoinHook(const std::string& channel, const std::string& user) {
	bot->modChanJoinHook(serverName, channel, user);
}

inline void Protocol::callChanPartHook(const std::string& channel, const std::string& user, const std::string& reason) {
	bot->modChanPartHook(serverName, channel, user, reason);
}

inline void Protocol::callChanKickHook(const std::string& channel, const std::string& kicker, const std::string& kickee, const std::string& reason) {
	bot->modChanKickHook(serverName, channel, kicker, kickee, reason);
}

inline void Protocol::callChanTopicHook(const std::string& channel, const std::string& nick, const std::string& topic) {
	bot->modChanTopicHook(serverName, channel, nick, topic);
}

inline void Protocol::callChanInviteHook(const std::string& channel, const std::string& inviter, const std::string& invitee) {
	bot->modChanInviteHook(serverName, channel, inviter, invitee);
}

inline void Protocol::callChanKnockHook(const std::string& channel, const std::string& nick, const std::string& reason) {
	bot->modChanKnockHook(serverName, channel, nick, reason);
}

inline void Protocol::callUserNickHook(const std::string& oldNick, const std::string& newNick) {
	bot->modUserNickHook(serverName, oldNick, newNick);
}

inline void Protocol::callServerPingHook(const std::string& sourceServer) {
	bot->modServerPingHook(serverName, sourceServer);
}

inline void Protocol::callServerPongHook(const std::string& sourceServer) {
	bot->modServerPongHook(serverName, sourceServer);
}

inline void Protocol::callNumericHook(const std::string& client, const std::string& numeric, const std::vector<std::string>& data) {
	bot->modNumericHook(serverName, client, numeric, data);
}

inline void Protocol::callUserOperHook(const std::string& nick, const std::string& operType) {
	bot->modUserOperHook(serverName, nick, operType);
}

inline void Protocol::callServerNoticeHook(char snomask, const std::string& message) {
	bot->modServerNoticeHook(serverName, snomask, message);
}

inline void Protocol::callMetadataHook(const std::string& target, const std::string& key, const std::string& value) {
	bot->modMetadataHook(serverName, target, key, value);
}

inline void Protocol::callXLineAddHook(const std::string& lineType, const std::string& mask) {
	bot->modXLineAddHook(serverName, lineType, mask);
}

inline void Protocol::callXLineRemoveHook(const std::string& lineType, const std::string& mask) {
	bot->modXLineRemoveHook(serverName, lineType, mask);
}

inline void Protocol::callUserConnectHook(const std::string& nick) {
	bot->modUserConnectHook(serverName, nick);
}

inline void Protocol::callUserQuitHook(const std::string& nick, const std::string& reason) {
	bot->modUserQuitHook(serverName, nick, reason);
}

inline void Protocol::callUserIdentChangeHook(const std::string& nick, const std::string& oldIdent, const std::string& newIdent) {
	bot->modUserIdentChangeHook(serverName, nick, oldIdent, newIdent);
}

inline void Protocol::callUserHostChangeHook(const std::string& nick, const std::string& oldHost, const std::string& newHost) {
	bot->modUserHostChangeHook(serverName, nick, oldHost, newHost);
}

inline void Protocol::callUserGecosChangeHook(const std::string& nick, const std::string& oldGecos, const std::string& newGecos) {
	bot->modUserGecosChangeHook(serverName, nick, oldGecos, newGecos);
}

inline void Protocol::callServerWallopsHook(const std::string& nick, const std::string& message) {
	bot->modServerWallopsHook(serverName, nick, message);
}

inline void Protocol::callServerConnectHook(const std::string& newServer) {
	bot->modServerConnectHook(serverName, newServer);
}

inline void Protocol::callServerDisconnectHook(const std::string& quitServer, const std::string& reason) {
	bot->modServerDisconnectHook(serverName, quitServer, reason);
}

inline void Protocol::callServerCapHook(const std::string& client, const std::string& subcmd, const std::string& list) {
	bot->modServerCapHook(serverName, client, subcmd, list);
}

inline void Protocol::callServerCapabHook(const std::vector<std::string>& capabList) {
	bot->modServerCapabHook(serverName, capabList);
}

inline void Protocol::callServerBurstHook() {
	bot->modServerBurstHook(serverName);
}

inline void Protocol::callServerBurstEndHook() {
	bot->modServerBurstEndHook(serverName);
}

inline void Protocol::callOtherDataHook(const std::string& client, const std::vector<std::string>& parsedLine) {
	bot->modOtherDataHook(serverName, client, parsedLine);
}

inline void Protocol::callChanMsgOutHook(const std::string& client, const std::string& channel, char status, const std::string& message) {
	bot->modChanMsgOutHook(serverName, client, channel, status, message, this);
}

inline void Protocol::callChanMsgSendHook(const std::string& client, const std::string& channel, char status, const std::string& message) {
	bot->modChanMsgSendHook(serverName, client, channel, status, message);
}

inline void Protocol::callUserMsgOutHook(const std::string& client, const std::string& nick, const std::string& message) {
	bot->modUserMsgOutHook(serverName, client, nick, message, this);
}

inline void Protocol::callUserMsgSendHook(const std::string& client, const std::string& nick, const std::string& message) {
	bot->modUserMsgSendHook(serverName, client, nick, message);
}

inline void Protocol::callChanNoticeOutHook(const std::string& client, const std::string& channel, char status, const std::string& message) {
	bot->modChanNoticeOutHook(serverName, client, channel, status, message, this);
}

inline void Protocol::callChanNoticeSendHook(const std::string& client, const std::string& channel, char status, const std::string& message) {
	bot->modChanNoticeSendHook(serverName, client, channel, status, message);
}

inline void Protocol::callUserNoticeOutHook(const std::string& client, const std::string& nick, const std::string& message) {
	bot->modUserNoticeOutHook(serverName, client, nick, message, this);
}

inline void Protocol::callUserNoticeSendHook(const std::string& client, const std::string& nick, const std::string& message) {
	bot->modUserNoticeSendHook(serverName, client, nick, message);
}

inline void Protocol::callChanCTCPOutHook(const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params) {
	bot->modChanCTCPOutHook(serverName, client, channel, status, ctcp, params, this);
}

inline void Protocol::callChanCTCPSendHook(const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params) {
	bot->modChanCTCPSendHook(serverName, client, channel, status, ctcp, params);
}

inline void Protocol::callUserCTCPOutHook(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) {
	bot->modUserCTCPOutHook(serverName, client, nick, ctcp, params, this);
}

inline void Protocol::callUserCTCPSendHook(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) {
	bot->modUserCTCPSendHook(serverName, client, nick, ctcp, params);
}

inline void Protocol::callChanCTCPReplyOutHook(const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params) {
	bot->modChanCTCPReplyOutHook(serverName, client, channel, status, ctcp, params, this);
}

inline void Protocol::callChanCTCPReplySendHook(const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params) {
	bot->modChanCTCPReplySendHook(serverName, client, channel, status, ctcp, params);
}

inline void Protocol::callUserCTCPReplyOutHook(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) {
	bot->modUserCTCPReplyOutHook(serverName, client, nick, ctcp, params, this);
}

inline void Protocol::callUserCTCPReplySendHook(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) {
	bot->modUserCTCPReplySendHook(serverName, client, nick, ctcp, params);
}