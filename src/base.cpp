#include "base.h"

Base::Base(std::string confdir, std::string confname, unsigned short debug) : debugLevel(debug), directory(confdir), configName(confname) {
	pthread_attr_init(&detachedState);
	pthread_attr_setdetachstate(&detachedState, PTHREAD_CREATE_DETACHED);
	pthread_create(&serverCheckThread, &detachedState, serverCheck_thread, this); // start thread that checks for disconnected servers
	ConfigReader config (confname, confdir);
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > serverConf = config.servers(true);
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > moduleConf = config.modules(true);
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator modConfIter = moduleConf.begin(); modConfIter != moduleConf.end(); ++modConfIter) {
		moduleConfigs.insert(std::pair<std::string, std::tr1::unordered_map<std::string, std::string> > (modConfIter->first, modConfIter->second));
		if (loadModule(modConfIter->first, true)) {
			if (debugLevel >= 2)
				std::cout << "Module " << modConfIter->first << " loaded successfully." << std::endl;
		} else
			std::cout << "Module " << modConfIter->first << " failed to load." << std::endl; // debug level 1
	}
	
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onLoadComplete(); // call the onLoadComplete hook in modules when all modules are loaded
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onLoadComplete();
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onLoadComplete();
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onLoadComplete();
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onLoadComplete();
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator servConfIter = serverConf.begin(); servConfIter != serverConf.end(); ++servConfIter) {
		serverConfigs.insert(std::pair<std::string, std::tr1::unordered_map<std::string, std::string> > (servConfIter->first, servConfIter->second));
		connectServer(servConfIter->first);
	}
	
	moduleConf = config.modules(false);
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator modConfIter = moduleConf.begin(); modConfIter != moduleConf.end(); ++modConfIter)
		moduleConfigs.insert(std::pair<std::string, std::tr1::unordered_map<std::string, std::string> > (modConfIter->first, modConfIter->second));
	serverConf = config.servers(false);
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator servConfIter = serverConf.begin(); servConfIter != serverConf.end(); ++servConfIter)
		serverConfigs.insert(std::pair<std::string, std::tr1::unordered_map<std::string, std::string> > (servConfIter->first, servConfIter->second));
}

std::tr1::unordered_map<std::string, std::string> Base::serverData(std::string server) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator serverIter = servers.find(server);
	if (serverIter == servers.end())
		return std::tr1::unordered_map<std::string, std::string> (); // a blank map for a nonexistent server
	return serverIter->second->info();
}

std::vector<std::vector<char> > Base::serverChanModes(std::string server) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator serverIter = servers.find(server);
	if (serverIter == servers.end())
		return std::vector<std::vector<char> > (); // Empty structure for whoever can't check the server list for real servers
	return serverIter->second->channelModes();
}

std::list<std::pair<char, char> > Base::serverPrefixes(std::string server) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator serverIter = servers.find(server);
	if (serverIter == servers.end())
		return std::list<std::pair<char, char> > ();
	return serverIter->second->prefixes();
}

bool Base::callChanMsgHook(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message) {
	bool keepGoing = true;
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onChannelMsg(server, client, channel, target, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onChannelMsg(server, client, channel, target, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onChannelMsg(server, client, channel, target, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onChannelMsg(server, client, channel, target, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onChannelMsg(server, client, channel, target, nick, message);
	return keepGoing;
}

bool Base::callUserMsgHook(std::string server, std::string client, std::string nick, std::string message) {
	bool keepGoing = true;
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onUserMsg(server, client, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onUserMsg(server, client, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onUserMsg(server, client, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onUserMsg(server, client, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onUserMsg(server, client, nick, message);
	return keepGoing;
}

bool Base::callChanNoticeHook(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message) {
	bool keepGoing = true;
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onChannelNotice(server, client, channel, target, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onChannelNotice(server, client, channel, target, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onChannelNotice(server, client, channel, target, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onChannelNotice(server, client, channel, target, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onChannelNotice(server, client, channel, target, nick, message);
	return keepGoing;
}

bool Base::callUserNoticeHook(std::string server, std::string client, std::string nick, std::string message) {
	bool keepGoing = true;
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onUserNotice(server, client, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onUserNotice(server, client, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onUserNotice(server, client, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onUserNotice(server, client, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onUserNotice(server, client, nick, message);
	return keepGoing;
}

bool Base::callChannelCTCPHook(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message) {
	bool keepGoing = true;
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onChannelCTCP(server, client, channel, target, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onChannelCTCP(server, client, channel, target, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onChannelCTCP(server, client, channel, target, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onChannelCTCP(server, client, channel, target, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onChannelCTCP(server, client, channel, target, nick, message);
	return keepGoing;
}

bool Base::callUserCTCPHook(std::string server, std::string client, std::string nick, std::string message) {
	bool keepGoing = true;
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onUserCTCP(server, client, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onUserCTCP(server, client, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onUserCTCP(server, client, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onUserCTCP(server, client, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onUserCTCP(server, client, nick, message);
	return keepGoing;
}

bool Base::callChannelCTCPReplyHook(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message) {
	bool keepGoing = true;
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onChannelCTCPReply(server, client, channel, target, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onChannelCTCPReply(server, client, channel, target, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onChannelCTCPReply(server, client, channel, target, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onChannelCTCPReply(server, client, channel, target, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onChannelCTCPReply(server, client, channel, target, nick, message);
	return keepGoing;
}

bool Base::callUserCTCPReplyHook(std::string server, std::string client, std::string nick, std::string message) {
	bool keepGoing = true;
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onUserCTCPReply(server, client, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onUserCTCPReply(server, client, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onUserCTCPReply(server, client, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onUserCTCPReply(server, client, nick, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end() && keepGoing; ++modIter)
		keepGoing = modIter->second->onUserCTCPReply(server, client, nick, message);
	return keepGoing;
}

void Base::callChannelJoinPreHook(std::string server, std::string channel, std::string hostmask) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onChannelJoinPre(server, channel, hostmask);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onChannelJoinPre(server, channel, hostmask);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onChannelJoinPre(server, channel, hostmask);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onChannelJoinPre(server, channel, hostmask);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onChannelJoinPre(server, channel, hostmask);
}

void Base::callChannelJoinPostHook(std::string server, std::string channel, std::string hostmask) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onChannelJoinPost(server, channel, hostmask);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onChannelJoinPost(server, channel, hostmask);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onChannelJoinPost(server, channel, hostmask);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onChannelJoinPost(server, channel, hostmask);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onChannelJoinPost(server, channel, hostmask);
}

void Base::callChannelPartPreHook(std::string server, std::string channel, std::string hostmask, std::string reason) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onChannelPartPre(server, channel, hostmask, reason);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onChannelPartPre(server, channel, hostmask, reason);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onChannelPartPre(server, channel, hostmask, reason);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.begin(); ++modIter)
		modIter->second->onChannelPartPre(server, channel, hostmask, reason);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onChannelPartPre(server, channel, hostmask, reason);
}

void Base::callChannelPartPostHook(std::string server, std::string channel, std::string hostmask, std::string reason) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onChannelPartPost(server, channel, hostmask, reason);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onChannelPartPost(server, channel, hostmask, reason);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onChannelPartPost(server, channel, hostmask, reason);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onChannelPartPost(server, channel, hostmask, reason);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onChannelPartPost(server, channel, hostmask, reason);
}

void Base::callUserConnectPreHook(std::string server, std::string nick, std::string ident, std::string host, std::string gecos) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onUserConnectPre(server, nick, ident, host, gecos);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onUserConnectPre(server, nick, ident, host, gecos);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onUserConnectPre(server, nick, ident, host, gecos);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onUserConnectPre(server, nick, ident, host, gecos);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onUserConnectPre(server, nick, ident, host, gecos);
}

void Base::callUserConnectPostHook(std::string server, std::string nick, std::string ident, std::string host, std::string gecos) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onUserConnectPost(server, nick, ident, host, gecos);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onUserConnectPost(server, nick, ident, host, gecos);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onUserConnectPost(server, nick, ident, host, gecos);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onUserConnectPost(server, nick, ident, host, gecos);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onUserConnectPost(server, nick, ident, host, gecos);
}

void Base::callUserQuitPreHook(std::string server, std::string hostmask, std::string reason) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onUserQuitPre(server, hostmask, reason);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onUserQuitPre(server, hostmask, reason);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onUserQuitPre(server, hostmask, reason);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onUserQuitPre(server, hostmask, reason);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onUserQuitPre(server, hostmask, reason);
}

void Base::callUserQuitPostHook(std::string server, std::string hostmask, std::string reason) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onUserQuitPost(server, hostmask, reason);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onUserQuitPost(server, hostmask, reason);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onUserQuitPost(server, hostmask, reason);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onUserQuitPost(server, hostmask, reason);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onUserQuitPost(server, hostmask, reason);
}

void Base::callNickChangePreHook(std::string server, std::string oldNick, std::string newNick) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onNickChangePre(server, oldNick, newNick);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onNickChangePre(server, oldNick, newNick);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onNickChangePre(server, oldNick, newNick);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onNickChangePre(server, oldNick, newNick);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onNickChangePre(server, oldNick, newNick);
}

void Base::callNickChangePostHook(std::string server, std::string oldNick, std::string newNick) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onNickChangePost(server, oldNick, newNick);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onNickChangePost(server, oldNick, newNick);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onNickChangePost(server, oldNick, newNick);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onNickChangePost(server, oldNick, newNick);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onNickChangePost(server, oldNick, newNick);
}

void Base::callChannelKickPreHook(std::string server, std::string channel, std::string kicker, std::string kickee, std::string reason) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onChannelKickPre(server, channel, kicker, kickee, reason);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onChannelKickPre(server, channel, kicker, kickee, reason);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onChannelKickPre(server, channel, kicker, kickee, reason);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onChannelKickPre(server, channel, kicker, kickee, reason);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onChannelKickPre(server, channel, kicker, kickee, reason);
}

void Base::callChannelKickPostHook(std::string server, std::string channel, std::string kicker, std::string kickee, std::string reason) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onChannelKickPost(server, channel, kicker, kickee, reason);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onChannelKickPost(server, channel, kicker, kickee, reason);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onChannelKickPost(server, channel, kicker, kickee, reason);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onChannelKickPost(server, channel, kicker, kickee, reason);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onChannelKickPost(server, channel, kicker, kickee, reason);
}

void Base::callChannelModePreHook(std::string server, std::string channel, std::string setter, std::string mode, bool add, std::string param) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onChannelModePre(server, channel, setter, mode, add, param);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onChannelModePre(server, channel, setter, mode, add, param);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onChannelModePre(server, channel, setter, mode, add, param);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onChannelModePre(server, channel, setter, mode, add, param);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onChannelModePre(server, channel, setter, mode, add, param);
}

void Base::callChannelModePostHook(std::string server, std::string channel, std::string setter, std::string mode, bool add, std::string param) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onChannelModePost(server, channel, setter, mode, add, param);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onChannelModePost(server, channel, setter, mode, add, param);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onChannelModePost(server, channel, setter, mode, add, param);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onChannelModePost(server, channel, setter, mode, add, param);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onChannelModePost(server, channel, setter, mode, add, param);
}

void Base::callUserModePreHook(std::string server, std::string client, std::string mode, bool add) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onUserModePre(server, client, mode, add);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onUserModePre(server, client, mode, add);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onUserModePre(server, client, mode, add);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onUserModePre(server, client, mode, add);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onUserModePre(server, client, mode, add);
}

void Base::callUserModePostHook(std::string server, std::string client, std::string mode, bool add) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onUserModePost(server, client, mode, add);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onUserModePost(server, client, mode, add);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onUserModePost(server, client, mode, add);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onUserModePost(server, client, mode, add);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onUserModePost(server, client, mode, add);
}

void Base::callUserOperPreHook(std::string server, std::string user, std::string opertype) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onUserOperPre(server, user, opertype);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onUserOperPre(server, user, opertype);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onUserOperPre(server, user, opertype);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onUserOperPre(server, user, opertype);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onUserOperPre(server, user, opertype);
}

void Base::callUserOperPostHook(std::string server, std::string user, std::string opertype) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onUserOperPost(server, user, opertype);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onUserOperPost(server, user, opertype);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onUserOperPost(server, user, opertype);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onUserOperPost(server, user, opertype);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onUserOperPost(server, user, opertype);
}

void Base::callNumericHook(std::string server, std::string client, std::string numeric, std::vector<std::string> parsedLine) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onNumeric(server, client, numeric, parsedLine);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onNumeric(server, client, numeric, parsedLine);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onNumeric(server, client, numeric, parsedLine);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onNumeric(server, client, numeric, parsedLine);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onNumeric(server, client, numeric, parsedLine);
}

void Base::callOtherDataHook(std::string server, std::string client, std::vector<std::string> parsedLine) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onOtherData(server, client, parsedLine);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onOtherData(server, client, parsedLine);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onOtherData(server, client, parsedLine);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onOtherData(server, client, parsedLine);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onOtherData(server, client, parsedLine);
}

void Base::callPreConnectHook(std::string server) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onPreConnect(server);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onPreConnect(server);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onPreConnect(server);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onPreConnect(server);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onPreConnect(server);
}

void Base::callConnectHook(std::string server, std::string client) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onConnect(server, client);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onConnect(server, client);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onConnect(server, client);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onConnect(server, client);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onConnect(server, client);
}

void Base::callQuitHook(std::string server, std::string client) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onQuit(server, client);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onQuit(server, client);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onQuit(server, client);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onQuit(server, client);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onQuit(server, client);
}

std::string Base::callChannelMessageOutHook(std::string server, std::string client, std::string target, char status, std::string message) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end() && message != ""; ++modIter)
		message = modIter->second->onChannelMessageOut(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end() && message != ""; ++modIter)
		message = modIter->second->onChannelMessageOut(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end() && message != ""; ++modIter)
		message = modIter->second->onChannelMessageOut(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end() && message != ""; ++modIter)
		message = modIter->second->onChannelMessageOut(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end() && message != ""; ++modIter)
		message = modIter->second->onChannelMessageOut(server, client, target, status, message);
	return message;
}

void Base::callChannelMessageSendHook(std::string server, std::string client, std::string target, char status, stdd::string message) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onChannelMessageSend(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onChannelMessageSend(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onChannelMessageSend(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onChannelMessageSend(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onChannelMessageSend(server, client, target, status, message);
}

std::string Base::callUserMessageOutHook(std::string server, std::string client, std::string target, std::string message) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end() && message != ""; ++modIter)
		message = modIter->second->onUserMessageOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end() && message != ""; ++modIter)
		message = modIter->second->onUserMessageOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end() && message != ""; ++modIter)
		message = modIter->second->onUserMessageOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end() && message != ""; ++modIter)
		message = modIter->second->onUserMessageOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end() && message != ""; ++modIter)
		message = modIter->second->onUserMessageOut(server, client, target, message);
	return message;
}

void Base::callUserMessageSendHook(std::string server, std::string client, std::string target, std::string message) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onUserMessageSend(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onUserMessageSend(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onUserMessageSend(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onUserMessageSend(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onUserMessageSend(server, client, target, message);
}

std::string Base::callChannelNoticeOutHook(std::string server, std::string client, std::string target, char status, std::string message) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end() && message != ""; ++modIter)
		message = modIter->second->onChannelNoticeOut(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end() && message != ""; ++modIter)
		message = modIter->second->onChannelNoticeOut(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end() && message != ""; ++modIter)
		message = modIter->second->onChannelNoticeOut(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end() && message != ""; ++modIter)
		message = modIter->second->onChannelNoticeOut(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end() && message != ""; ++modIter)
		message = modIter->second->onChannelNoticeOut(server, client, target, status, message);
	return message;
}

void Base::callChannelNoticeSendHook(std::string server, std::string client, std::string target, char status, std::string message) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onChannelNoticeSend(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onChannelNoticeSend(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onChannelNoticeSend(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onChannelNoticeSend(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onChannelNoticeSend(server, client, target, status, message);
}

std::string Base::callUserNoticeOutHook(std::string server, std::string client, std::string target, std::string message) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end() && message != ""; ++modIter)
		message = modIter->second->onUserNoticeOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end() && message != ""; ++modIter)
		message = modIter->second->onUserNoticeOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end() && message != ""; ++modIter)
		message = modIter->second->onUserNoticeOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end() && message != ""; ++modIter)
		message = modIter->second->onUserNoticeOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end() && message != ""; ++modIter)
		message = modIter->second->onUserNoticeOut(server, client, target, message);
	return message;
}

void Base::callUserNoticeSendHook(std::string server, std::string client, std::string target, std::string message) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onUserNoticeOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onUserNoticeOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onUserNoticeOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onUserNoticeOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onUserNoticeOut(server, client, target, message);
}

std::string Base::callChannelCTCPOutHook(std::string server, std::string client, std::string target, char status, std::string message) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end() && message != ""; ++modIter)
		message = modIter->second->onChannelCTCPOut(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end() && message != ""; ++modIter)
		message = modIter->second->onChannelCTCPOut(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end() && message != ""; ++modIter)
		message = modIter->second->onChannelCTCPOut(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end() && message != ""; ++modIter)
		message = modIter->second->onChannelCTCPOut(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end() && message != ""; ++modIter)
		message = modIter->second->onChannelCTCPOut(server, client, target, status, message);
	return message;
}

void Base::callChannelCTCPSendHook(std::string server, std::string client, std::string target, char status, std::string message) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onChannelCTCPOut(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onChannelCTCPOut(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onChannelCTCPOut(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onChannelCTCPOut(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onChannelCTCPOut(server, client, target, status, message);
}

std::string Base::callUserCTCPOutHook(std::string server, std::string client, std::string target, std::string message) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end() && message != ""; ++modIter)
		message = modIter->second->onUserCTCPOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end() && message != ""; ++modIter)
		message = modIter->second->onUserCTCPOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end() && message != ""; ++modIter)
		message = modIter->second->onUserCTCPOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end() && message != ""; ++modIter)
		message = modIter->second->onUserCTCPOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end() && message != ""; ++modIter)
		message = modIter->second->onUserCTCPOut(server, client, target, message);
	return message;
}

void Base::callUserCTCPSendHook(std::string server, std::string client, std::string target, std::string message) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onUserCTCPOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onUserCTCPOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onUserCTCPOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onUserCTCPOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onUserCTCPOut(server, client, target, message);
}

std::string Base::callChannelCTCPReplyOutHook(std::string server, std::string client, std::string target, char status, std::string message) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end() && message != ""; ++modIter)
		message = modIter->second->onChannelCTCPReplyOut(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end() && message != ""; ++modIter)
		message = modIter->second->onChannelCTCPReplyOut(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end() && message != ""; ++modIter)
		message = modIter->second->onChannelCTCPReplyOut(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end() && message != ""; ++modIter)
		message = modIter->second->onChannelCTCPReplyOut(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end() && message != ""; ++modIter)
		message = modIter->second->onChannelCTCPReplyOut(server, client, target, status, message);
	return message;
}

void Base::callChannelCTCPReplySendHook(std::string server, std::string client, std::string target, char status, std::string message) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onChannelCTCPReplyOut(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onChannelCTCPReplyOut(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onChannelCTCPReplyOut(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onChannelCTCPReplyOut(server, client, target, status, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onChannelCTCPReplyOut(server, client, target, status, message);
}

std::string Base::callUserCTCPReplyOutHook(std::string server, std::string client, std::string target, std::string message) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end() && message != ""; ++modIter)
		message = modIter->second->onUserCTCPReplyOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end() && message != ""; ++modIter)
		message = modIter->second->onUserCTCPReplyOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end() && message != ""; ++modIter)
		message = modIter->second->onUserCTCPReplyOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end() && message != ""; ++modIter)
		message = modIter->second->onUserCTCPReplyOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end() && message != ""; ++modIter)
		message = modIter->second->onUserCTCPReplyOut(server, client, target, message);
	return message;
}

void Base::callUserCTCPReplySendHook(std::string server, std::string client, std::string target, std::string message) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modIter->second->onUserCTCPReplyOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modIter->second->onUserCTCPReplyOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modIter->second->onUserCTCPReplyOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modIter->second->onUserCTCPReplyOut(server, client, target, message);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modIter->second->onUserCTCPReplyOut(server, client, target, message);
}

void Base::sendPrivMsg(std::string server, std::string client, std::string target, std::string message) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->sendMsg(client, target, message);
}

void Base::sendNotice(std::string server, std::string client, std::string target, std::string message) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->sendNotice(client, target, message);
}

void Base::setMode(std::string server, std::string client, std::string target, std::string mode) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->setMode(client, target, mode);
}

void Base::removeMode(std::string server, std::string client, std::string target, std::string mode) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->removeMode(client, target, mode);
}

void Base::joinChannel(std::string server, std::string client, std::string channel, std::string key) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->joinChannel(client, channel, key);
}

void Base::partChannel(std::string server, std::string client, std::string channel, std::string reason) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->partChannel(client, channel, reason);
}

void Base::quitServer(std::string server, std::string reason) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->quitServer(reason);
}

void Base::kickUser(std::string server, std::string client, std::string channel, std::string user, std::string reason) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->kickUser(client, channel, user, reason);
}

void Base::changeNick(std::string server, std::string client, std::string newNick) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->changeNick(client, newNick);
}

void Base::oper(std::string server, std::string client, std::string username, std::string password) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->oper(client, username, password, opertype);
}

void Base::killUser(std::string server, std::string client, std::string user, std::string reason) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->killUser(client, user, reason);
}

void Base::setXLine(std::string server, std::string client, char lineType, std::string hostmask, time_t duration, std::string reason) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->setXLine(client, lineType, hostmask, duration, reason);
}

void Base::removeXLine(std::string server, std::string client, char lineType, std::string hostmask) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->removeXLine(client, lineType, hostmask);
}

std::tr1::unordered_map<char, std::tr1::unordered_map<std::string, time_t> > Base::listXLines(std::string server) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::tr1::unordered_map<char, std::tr1::unordered_map<std::string, time_t> > ();
	return servIter->second->listXLines();
}

void Base::sendSNotice(std::string server, char snomask, std::string text) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->sendSNotice(snomask, text);
}

void Base::sendOther(std::string server, std::string rawLine) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->sendOther(rawLine);
}

std::string Base::addClient(std::string server, std::string nick, std::string ident, std::string host, std::string gecos) {
	if (servers.find(server) == servers.end())
		return;
	return servers.find(server)->second->addClient(nick, ident, host, gecos);
}

void Base::removeClient(std::string server, std::string client, std::string reason) {
	if (servers.find(server) == servers.end())
		return;
	servers.find(server)->second->removeClient(client, reason);
}

bool Base::isChanType(char chanPrefix, std::string server) {
	if (servers.find(server) == servers.end())
		return false;
	std::vector<char> prefixes = servers.find(server)->second->channelTypes();
	for (unsigned int i = 0; i < prefixes.size(); i++) {
		if (chanPrefix == prefixes[i])
			return true;
	}
	return false;
}

std::list<std::string> Base::serverList() {
	std::list<std::string> listOfServers;
	for (std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.begin(); servIter != servers.end(); ++servIter)
		listOfServers.insert(listOfServers.end(), servIter->first);
	return listOfServers;
}

std::tr1::unordered_map<std::string, Module*> Base::loadedModules() {
	std::tr1::unordered_map<std::string, Module*> modules;
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter)
		modules.insert(std::pair<std::string, Module*> (modIter->first, modIter->second));
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modules.insert(std::pair<std::string, Module*> (modIter->first, modIter->second));
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modules.insert(std::pair<std::string, Module*> (modIter->first, modIter->second));
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modules.insert(std::pair<std::string, Module*> (modIter->first, modIter->second));
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modules.insert(std::pair<std::string, Module*> (modIter->first, modIter->second));
	return modules;
}

std::multimap<std::string, std::string> Base::moduleAbilities() {
	return modAbilities;
}

std::tr1::unordered_map<std::string, std::vector<std::string> > Base::moduleSupports() {
	return modSupports;
}

bool Base::serverIsClient(std::string server) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return false;
	return servIter->second->isClient();
}

std::list<std::string> Base::channels(std::string server) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> ();
	return servIter->second->channels();
}

std::string Base::channelTopic(std::string server, std::string channel) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return "";
	return servIter->second->channelTopic(channel);
}

std::list<std::string> Base::channelUsers(std::string server, std::string channel) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> (); // return empty list to those who cannot provide a valid server name
	return servIter->second->channelUsers(channel);
}

std::string Base::userIdent(std::string server, std::string user) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return "";
	return servIter->second->userIdent(user);
}

std::string Base::userHost(std::string server, std::string user) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return "";
	return servIter->second->userHost(user);
}

std::pair<std::string, char> Base::userStatus(std::string server, std::string channel, std::string user) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::pair<char, char> ("", ' '); // pair for normal user
	return servIter->second->userStatus(channel, user);
}

Socket* Base::assignSocket(std::string socketType) {
	void* openSocket;
	if (socketFiles.find(socketType) == socketFiles.end()) {
		std::string fileLoc = directory + "/modules/s_" + socketType + ".so";
		openSocket = dlopen(fileLoc.c_str(), RTLD_NOW);
		if (openSocket == NULL) {
			std::string error = "Could not open socket type " + socketType + ": " + dlerror();
			std::perror(error.c_str()); // debug level 1
			return NULL;
		}
	} else
		openSocket = socketFiles.find(socketType)->second;
	socket_spawn_t spawnSocket = (socket_spawn_t) dlsym(openSocket, "spawn");
	const char* dlsymError = dlerror();
	if (dlsymError) {
		std::string error = "Could not load socket type " + socketType + ": " + dlsymError;
		std::perror(error.c_str()); // debug level 1
		return NULL;
	}
	
	Socket* newSocket = (Socket*) spawnSocket();
	if (newSocket->apiVersion() != 2000) { // compare to current API version
		dlclose(openSocket);
		std::cout << "The socket type " << socketType << " that attempted to load is not compatible with this version of RoBoBo." << std::endl;
		return NULL;
	}
	socketFiles.insert(std::pair<std::string, void*> (socketType, openSocket));
	return newSocket;
}

void Base::rehash() {
	ConfigReader config (configName, directory);
	serverConfigs.clear();
	serverConfigs = config.servers(true);
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > serverConf = config.servers(false);
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator servIter = serverConf.begin(); servIter != serverConf.end(); ++servIter)
		serverConfigs.insert(std::pair<std::string, std::tr1::unordered_map<std::string, std::string> > (servIter->first, servIter->second));
	moduleConfigs.clear();
	moduleConfigs = config.modules(true);
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > moduleConf = config.modules(false);
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator modIter = moduleConf.begin(); modIter != moduleConf.end(); ++modIter)
		moduleConfigs.insert(std::pair<std::string, std::tr1::unordered_map<std::string, std::string> > (modIter->first, modIter->second));
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator modIter = moduleConfigs.begin(); modIter != moduleConfigs.end(); ++modIter) {
		std::tr1::unordered_map<std::string, Module*>::iterator module = highModules.find(modIter->first);
		if (module != highModules.end()) {
			module->second->reconf(modIter->second);
			module->second->onRehash();
		} else {
			module = mediumHighModules.find(modIter->first);
			if (module != mediumHighModules.end()) {
				module->second->reconf(modIter->second);
				module->second->onRehash();
			} else {
				module = normalModules.find(modIter->first);
				if (module != normalModules.end()) {
					module->second->reconf(modIter->second);
					module->second->onRehash();
				} else {
					module = mediumLowModules.find(modIter->first);
					if (module != mediumLowModules.end()) {
						module->second->reconf(modIter->second);
						module->second->onRehash();
					} else {
						module = lowModules.find(modIter->first);
						if (module != lowModules.end()) {
							module->second->reconf(modIter->second);
							module->second->onRehash();
						}
					}
				}
			}
		}
	}
}

bool Base::connectServer(std::string serverName) {
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator servConfIter = serverConfigs.find(serverName);
	if (servConfIter == serverConfigs.end())
		return false;
	if (servConfIter->second["protocol"] == "")
		return false;
	void* protoFile;
	if (protocolFiles.find(servConfIter->second["protocol"]) == protocolFiles.end()) {
		std::string fileLoc = "/modules/p_" + servConfIter->second["protocol"] + ".so";
		protoFile = dlopen(fileLoc.c_str(), RTLD_NOW);
		if (protoFile == NULL) {
			std::string error = "Could not open protocol " + servConfIter->second["protocol"] + ": " + dlerror();
			std::perror(error.c_str()); // debug level 1
			return false;
		}
	} else
		protoFile = protocolFiles.find(servConfIter->second["protocol"])->second;
	proto_spawn_t spawnProto = (proto_spawn_t) dlsym(protoFile, "spawn");
	const char* dlsymError = dlerror();
	if (dlsymError) {
		std::string error = "Could not open protocol " + servConfIter->second["protocol"] + ": " + dlsymError;
		std::perror(error.c_str()); // debug level 1
		return false;
	}
	
	Protocol* newProto = (Protocol*) spawnProto(serverName, servConfIter->second, this, debugLevel);
	if (newProto->apiVersion() != 2000) { // compare to current API version
		dlclose(protoFile);
		std::cout << "The protocol type " << servConfIter->second["protocol"] << " that attempted to load is not compatible with this version of RoBoBo." << std::endl;
		return false;
	}
	protocolFiles.insert(std::pair<std::string, void*> (servConfIter->second["protocol"], protoFile));
	servers.insert(std::pair<std::string, Protocol*> (serverName, newProto));
	servers.find(serverName)->second->connectServer();
	return true;
}

bool Base::loadModule(std::string modName, bool startup) {
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator modConf = moduleConfigs.find(modName);
	if (modConf == moduleConfigs.end()) { // give module a blank config and let the module reject it if it wants
		moduleConfigs.insert(std::pair<std::string, std::tr1::unordered_map<std::string, std::string> > (modName, std::tr1::unordered_map<std::string, std::string> ()));
		modConf = moduleConfigs.find(modName);
	}
	std::string fileLoc = directory + "/modules/m_" + modName + ".so";
	void* openModule = dlopen(fileLoc.c_str(), RTLD_NOW);
	if (openModule == NULL) {
		std::string error = "Could not open module " + modName + ": " + dlerror();
		std::perror(error.c_str()); // debug level 1
		return false;
	}
	module_spawn_t spawnModule = (module_spawn_t) dlsym(openModule, "spawn");
	const char* dlsymError = dlerror();
	if (dlsymError) {
		std::string error = "Could not load module " + modName + ": " + dlsymError;
		std::perror(error.c_str()); // debug level 1
		return false;
	}
	
	Module* newModule = (Module*) spawnModule(modConf->second, this, modName, directory, debugLevel);
	if (newModule->botAPIversion() != 2000) { // compare to current API version
		dlclose(openModule);
		std::cout << "Module " << modName << " is not compatible with this version of RoBoBo." << std::endl; // debug level 1
		return false;
	}
	switch (newModule->receivePriority()) {
		case HIGH:
			highModules.insert(std::pair<std::string, Module*> (modName, newModule));
			break;
		case MEDIUM_HIGH:
			mediumHighModules.insert(std::pair<std::string, Module*> (modName, newModule));
			break;
		case NORMAL:
			normalModules.insert(std::pair<std::string, Module*> (modName, newModule));
			break;
		case MEDIUM_LOW:
			mediumLowModules.insert(std::pair<std::string, Module*> (modName, newModule));
			break;
		case LOW:
			lowModules.insert(std::pair<std::string, Module*> (modName, newModule));
	}
	moduleFiles.insert(std::pair<std::string, void*> (modName, openModule));
	std::vector<std::string> abilities = newModule->abilities();
	for (unsigned int i = 0; i < abilities.size(); i++)
		modAbilities.insert(std::pair<std::string, std::string> (abilities[i], modName));
	std::vector<std::string> supports = newModule->supports();
	for (unsigned int i = 0; i < supports.size(); i++)
		modSupports[supports[i]].push_back(modName);
	if (!startup) {
		if (newModule->onLoadComplete()) {
			for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = highModules.begin(); modIter != highModules.end(); ++modIter) {
				if (modIter->first != modName)
					modIter->second->onModuleChange();
			}
			for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter) {
				if (modIter->first != modName)
					modIter->second->onModuleChange();
			}
			for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter) {
				if (modIter->first != modName)
					modIter->second->onModuleChange();
			}
			for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter) {
				if (modIter->first != modName)
					modIter->second->onModuleChange();
			}
			for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter) {
				if (modIter->first != modName)
					modIter->second->onModuleChange();
			}
			return true;
		} else
			return false;
	}
	return true;
}

void Base::unloadModule(std::string modName) {
	moduleToUnload.push_back(modName);
	pthread_t tum;
	pthread_create(&tum, NULL, tUnloadMod_thread, this);
}

void* Base::serverCheck_thread(void* ptr) {
	Base* modi = (Base*) ptr;
	modi->serverCheck();
	return NULL;
}

void Base::serverCheck() {
	while (true) {
		sleep(60); // one minute pause between checks
		for (std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.begin(); servIter != servers.end(); ++servIter) {
			if (!servIter->second->stillConnected()) {
				bool restartServer = servIter->second->shouldReset();
				delete servIter->second;
				std::string serverName = servIter->first;
				servers.erase(servIter);
				if (debugLevel >= 2)
					std::cout << serverName << " lost connection.  Reconnecting..." << std::endl;
				if (restartServer)
					connectServer(serverName); // make new server for reconnecting
				else
					servers.erase(servIter);
			}
		}
	}
}

bool Base::charIsNumeric(char number) {
	if (number == '0' || number == '1' || number == '2' || number == '3' || number == '4' || number == '5' || number == '6' || number == '7' || number == '8' || number == '9')
		return true;
	return false;
}

void* Base::tUnloadMod_thread(void* mip) {
	Base* modi = (Base*) mip;
	modi->tUnloadMod();
	return NULL;
}

void Base::tUnloadMod() {
	if (moduleToUnload.empty())
		return;
	sleep(1);
	std::tr1::unordered_map<std::string, Module*>* modules;
	if (highModules.find(moduleToUnload[0]) != highModules.end())
		modules = &highModules;
	else if (mediumHighModules.find(moduleToUnload[0]) != mediumHighModules.end())
		modules = &mediumHighModules;
	else if (normalModules.find(moduleToUnload[0]) != normalModules.end())
		modules = &normalModules;
	else if (mediumLowModules.find(moduleToUnload[0]) != mediumLowModules.end())
		modules = &mediumLowModules;
	else if (lowModules.find(moduleToUnload[0]) != lowModules.end())
		modules = &lowModules;
	else
		return;
	std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->find(moduleToUnload[0]);
	std::tr1::unordered_map<std::string, void*>::iterator modFileIter = moduleFiles.find(moduleToUnload[0]);
	if (!modIter->second->abilities().empty()) {
		for (std::multimap<std::string, std::string>::iterator modAbleIter = modAbilities.begin(); modAbleIter != modAbilities.end(); ++modAbleIter) {
			if (modAbleIter->second == modIter->first) {
				modAbilities.erase(modAbleIter);
				--modAbleIter;
			}
		}
	}
	if (!modIter->second->supports().empty()) {
		for (std::tr1::unordered_map<std::string, std::vector<std::string> >::iterator modSuppIter = modSupports.begin(); modSuppIter != modSupports.end(); ++modSuppIter) {
			for (unsigned int i = 0; i < modSuppIter->second.size(); i++) {
				if (modSuppIter->second[i] == modIter->first) {
					modSuppIter->second.erase(modSuppIter->second.begin() + i);
					break;
				}
			}
		}
	}
	delete modIter->second;
	modules->erase(modIter);
	dlclose(modFileIter->second);
	moduleFiles.erase(modFileIter);
	for (std::tr1::unordered_map<std::string, Module*>::iterator moduleIter = modules->begin(); moduleIter != modules->end(); ++moduleIter)
		moduleIter->second->onModuleChange(); // provide modules with a way to detect unloading
	moduleToUnload.erase(moduleToUnload.begin()); // remove first element, the one we just removed
}