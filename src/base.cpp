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

void Base::sendToServer(std::string server, std::string rawLine) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator serverIter = servers.find(server);
	if (serverIter == servers.end())
		return;
	serverIter->second->sendLine(rawLine); // change to suit new protocol function format
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

std::vector<std::pair<char, char> > Base::serverPrefixes(std::string server) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator serverIter = servers.find(server);
	if (serverIter == servers.end())
		return std::vector<std::pair<char, char> > ();
	return serverIter->second->prefixes();
}

void Base::callPreHook(std::string server, std::vector<std::string> parsedLine) {
	callPreModulesHook(server, parsedLine, &highModules);
	callPreModulesHook(server, parsedLine, &mediumHighModules);
	callPreModulesHook(server, parsedLine, &normalModules);
	callPreModulesHook(server, parsedLine, &mediumLowModules);
	callPreModulesHook(server, parsedLine, &lowModules);
}

void Base::callPostHook(std::string server, std::vector<std::string> parsedLine) {
	callPostModulesHook(server, parsedLine, &highModules);
	callPostModulesHook(server, parsedLine, &mediumHighModules);
	callPostModulesHook(server, parsedLine, &normalModules);
	callPostModulesHook(server, parsedLine, &mediumLowModules);
	callPostModulesHook(server, parsedLine, &lowModules);
}

std::string Base::callHookOut(std::string server, std::vector<std::string> parsedLine) {
	std::string resultMessage = callHookOut(server, parsedLine, &highModules);
	if (resultMessage == "")
		return "";
	parsedLine[2] = resultMessage;
	resultMessage = callHookOut(server, parsedLine, &mediumHighModules);
	if (resultMessage == "")
		return "";
	parsedLine[2] = resultMessage;
	resultMessage = callHookOut(server, parsedLine, &normalModules);
	if (resultMessage == "")
		return "";
	parsedLine[2] = resultMessage;
	resultMessage = callHookOut(server, parsedLine, &mediumLowModules);
	if (resultMessage == "")
		return "";
	parsedLine[2] = resultMessage;
	resultMessage = callHookOut(server, parsedLine, &lowModules);
	return resultMessage;
}

void Base::callHookSend(std::string server, std::vector<std::string> parsedLine) {
	callModulesHookSend(server, parsedLine, &highModules);
	callModulesHookSend(server, parsedLine, &mediumHighModules);
	callModulesHookSend(server, parsedLine, &normalModules);
	callModulesHookSend(server, parsedLine, &mediumLowModules);
	callModulesHookSend(server, parsedLine, &lowModules);
}

void Base::callPreConnectHook(std::string server, std::vector<std::string> parsedLine) {
	callPreConnectModulesHook(server, parsedLine, &highModules);
	callPreConnectModulesHook(server, parsedLine, &mediumHighModules);
	callPreConnectModulesHook(server, parsedLine, &normalModules);
	callPreConnectModulesHook(server, parsedLine, &mediumLowModules);
	callPreConnectModulesHook(server, parsedLine, &lowModules);
}

void Base::callConnectHook(std::string server, std::vector<std::string> parsedLine) {
	callConnectModulesHook(server, parsedLine, &highModules);
	callConnectModulesHook(server, parsedLine, &mediumHighModules);
	callConnectModulesHook(server, parsedLine, &normalModules);
	callConnectModulesHook(server, parsedLine, &mediumLowModules);
	callConnectModulesHook(server, parsedLine, &lowModules);
}

void Base::callQuitHook(std::string server, std::vector<std::string> parsedLine) {
	callQuitModulesHook(server, parsedLine, &highModules);
	callQuitModulesHook(server, parsedLine, &mediumHighModules);
	callQuitModulesHook(server, parsedLine, &normalModules);
	callQuitModulesHook(server, parsedLine, &mediumLowModules);
	callQuitModulesHook(server, parsedLine, &lowModules);
}

void Base::callPreModulesHook(std::string server, std::vector<std::string> parsedLine, std::tr1::unordered_map<std::string, Module*>* modules) {
	if (parsedLine[1] == "JOIN") {
		std::string hostmask = parsedLine[0].substr(1);
		for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
			modIter->second->onChannelJoinPre(server, parsedLine[2], hostmask);
	} else if (parsedLine[1] == "PART") {
		std::string hostmask = parsedLine[0].substr(1);
		if (parsedLine.size() == 3) {
			for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
				modIter->second->onChannelPartPre(server, parsedLine[2], hostmask, "");
		} else {
			for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
				modIter->second->onChannelPartPre(server, parsedLine[2], hostmask, parsedLine[3]);
		}
	} else if (parsedLine[1] == "QUIT") {
		std::string hostmask = parsedLine[0].substr(1);
		if (parsedLine.size() == 2) {
			for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
				modIter->second->onUserQuitPre(server,hostmask, "");
		} else {
			for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
				modIter->second->onUserQuitPre(server, hostmask, parsedLine[2]);
		}
	} else if (parsedLine[1] == "NICK") {
		std::string nick = parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1);
		for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
			modIter->second->onNickChangePre(server, nick, parsedLine[2]);
	} else if (parsedLine[1] == "KICK") {
		std::string nick = parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1);
		for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
			modIter->second->onChannelKickPre(server, parsedLine[2], nick, parsedLine[3], parsedLine[4]);
	} else if (parsedLine[1] == "MODE") {
		bool addMode = true;
		int currParam = 4;
		for (unsigned int i = 0; i < parsedLine[3].size(); i++) {
			if (parsedLine[3][i] == '+')
				addMode = true;
			else if (parsedLine[3][i] == '-')
				addMode = false;
			else {
				std::vector<std::vector<char> > serverModes;
				std::vector<std::pair<char, char> > prefixes;
				std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
				serverModes = servIter->second->channelModes();
				prefixes = servIter->second->prefixes();
				short category = 0;
				bool found = false;
				for (unsigned int j = 0; j < prefixes.size(); j++) {
					if (prefixes[j].first == parsedLine[3][i]) {
						found = true;
						break;
					}
				}
				if (!found) {
					found = false;
					for (unsigned int j = 0; j < serverModes.size(); j++) {
						for (unsigned int k = 0; k < serverModes[j].size(); k++) {
							if (parsedLine[3][i] == serverModes[j][k]) {
								found = true;
								category = j;
								break;
							}
						}
						if (found)
							break;
					}
					if (!found)
						category = 4;
				}
				std::string from = parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1);
				if (category == 0 || category == 1 || (category == 2 && addMode)) {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
						modIter->second->onChannelModePre(server, parsedLine[2], from, parsedLine[3][i], addMode, parsedLine[currParam]);
					currParam++;
				} else {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
						modIter->second->onChannelModePre(server, parsedLine[2], from, parsedLine[3][i], addMode, "");
				}
			}
		}
	}
}

void Base::callPostModulesHook(std::string server, std::vector<std::string> parsedLine, std::tr1::unordered_map<std::string, Module*>* modules) {
	if (parsedLine[1] == "PRIVMSG") { // lots of things!
		std::string from = parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1);
		if (parsedLine[3][0] == (char)1) { // CTCP request
			parsedLine[3] = parsedLine[3].substr(1);
			if (parsedLine[3][parsedLine[3].size() - 1] == (char)1)
				parsedLine[3] = parsedLine[3].substr(0, parsedLine[3].size() - 1);
			if (parsedLine[3].substr(0, parsedLine[3].find_first_of(' ')) == "ACTION") { // CTCP ACTION
				std::string message = parsedLine[3].substr(7);
				if (isChanType(parsedLine[2][0], server)) {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
						modIter->second->onChannelAction(server, parsedLine[2], '0', from, message);
				} else if (isChanType(parsedLine[2][1], server)) {
					char status = parsedLine[2][0];
					std::string channel = parsedLine[2].substr(1);
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
						modIter->second->onChannelAction(server, channel, status, from, message);
				} else {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
						modIter->second->onUserAction(server, from, message);
				}
			} else { // CTCP but not ACTION
				if (isChanType(parsedLine[2][0], server)) {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
						modIter->second->onChannelCTCP(server, parsedLine[2], '0', from, parsedLine[3]);
				} else if (isChanType(parsedLine[2][1], server)) {
					char status = parsedLine[2][0];
					std::string channel = parsedLine[2].substr(1);
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
						modIter->second->onChannelCTCP(server, channel, status, from, parsedLine[3]);
				} else {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
						modIter->second->onUserCTCP(server, from, parsedLine[3]);
				}
			}
		} else { // it's a message!
			if (isChanType(parsedLine[2][0], server)) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
					modIter->second->onChannelMsg(server, parsedLine[2], '0', from, parsedLine[3]);
			} else if (isChanType(parsedLine[2][1], server)) {
				char status = parsedLine[2][0];
				std::string channel = parsedLine[2].substr(1);
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
					modIter->second->onChannelMsg(server, channel, status, from, parsedLine[3]);
			} else {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
					modIter->second->onUserMsg(server, from, parsedLine[3]);
			}
		}
	} else if (parsedLine[1] == "NOTICE") {
		std::string from = parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1);
		if (parsedLine[3][0] == (char)1) { // CTCP reply
			parsedLine[3] = parsedLine[3].substr(1);
			if (parsedLine[3][parsedLine[3].size() - 1] == (char)1)
				parsedLine[3] = parsedLine[3].substr(0, parsedLine[3].size() - 1);
			if (isChanType(parsedLine[2][0], server)) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
					modIter->second->onChannelCTCPReply(server, parsedLine[2], '0', from, parsedLine[3]);
			} else if (isChanType(parsedLine[2][1], server)) {
				char status = parsedLine[2][0];
				std::string channel = parsedLine[2].substr(1);
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
					modIter->second->onChannelCTCPReply(server, channel, status, from, parsedLine[3]);
			} else {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
					modIter->second->onUserCTCPReply(server, from, parsedLine[3]);
			}
		} else { // it's a notice!
			if (isChanType(parsedLine[2][0], server)) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
					modIter->second->onChannelNotice(server, parsedLine[2], '0', from, parsedLine[3]);
			} else if (isChanType(parsedLine[2][1], server)) {
				char status = parsedLine[2][0];
				std::string channel = parsedLine[2].substr(1);
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
					modIter->second->onChannelNotice(server, channel, status, from, parsedLine[3]);
			} else {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
					modIter->second->onUserNotice(server, from, parsedLine[3]);
			}
		}
	} else if (parsedLine[1] == "JOIN") {
		std::string hostmask = parsedLine[0].substr(1);
		for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
			modIter->second->onChannelJoinPost(server, parsedLine[2], hostmask);
	} else if (parsedLine[1] == "PART") {
		std::string hostmask = parsedLine[0].substr(1);
		if (parsedLine.size() == 3) {
			for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
				modIter->second->onChannelPartPost(server, parsedLine[2], hostmask, "");
		} else {
			for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
				modIter->second->onChannelPartPost(server, parsedLine[2], hostmask, parsedLine[3]);
		}
	} else if (parsedLine[1] == "QUIT") {
		std::string hostmask = parsedLine[0].substr(1);
		if (parsedLine.size() == 2) {
			for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
				modIter->second->onUserQuitPost(server, hostmask, "");
		} else {
			for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
				modIter->second->onUserQuitPost(server, hostmask, parsedLine[2]);
		}
	} else if (parsedLine[1] == "NICK") {
		std::string nick = parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1);
		for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
			modIter->second->onNickChangePost(server, nick, parsedLine[2]);
	} else if (parsedLine[1] == "KICK") {
		std::string kicker = parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1);
		for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
			modIter->second->onChannelKickPost(server, parsedLine[2], kicker, parsedLine[3], parsedLine[4]);
	} else if (parsedLine[1] == "MODE") {
		bool addMode = true;
		int currParam = 4;
		for (unsigned int i = 0; i < parsedLine[3].size(); i++) {
			if (parsedLine[3][i] == '+')
				addMode = true;
			else if (parsedLine[3][i] == '-')
				addMode = false;
			else {
				std::vector<std::vector<char> > serverModes;
				std::vector<std::pair<char, char> > prefixes;
				std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
				serverModes = servIter->second->channelModes();
				prefixes = servIter->second->prefixes();
				short category = 0;
				bool found = false;
				for (unsigned int j = 0; j < prefixes.size(); j++) {
					if (prefixes[i].first == parsedLine[3][i]) {
						found = true;
						break;
					}
				}
				if (!found) {
					found = false;
					for (unsigned int j = 0; j < serverModes.size(); j++) {
						for (unsigned int k = 0; k < serverModes[j].size(); k++) {
							if (parsedLine[3][i] == serverModes[j][k]) {
								found = true;
								category = j;
								break;
							}
						}
						if (found)
							break;
					}
					if (!found)
						category = 4;
				}
				std::string from = parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1);
				if (category == 0 || category == 1 || (category == 2 && addMode)) {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
						modIter->second->onChannelModePost(server, parsedLine[2], from, parsedLine[3][i], addMode, parsedLine[currParam]);
					currParam++;
				} else {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
						modIter->second->onChannelModePost(server, parsedLine[2], from, parsedLine[3][i], addMode, "");
				}
			}
		}
	} else if (parsedLine[1].size() == 3 && charIsNumeric(parsedLine[1][0]) && charIsNumeric(parsedLine[1][1]) && charIsNumeric(parsedLine[1][2])) {
		for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
			modIter->second->onNumeric(server, parsedLine[1], parsedLine);
	} else {
		for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
			modIter->second->onOtherData(server, parsedLine);
	}
}

std::string Base::callModulesHookOut(std::string server, std::vector<std::string> parsedLine, std::tr1::unordered_map<std::string, Module*>* modules) {
	std::string message = parsedLine[2];
	if (parsedLine[0] == "PRIVMSG") {
		if (message[0] == (char)1) {
			if (message.substr(1, 6) == "ACTION") {
				message = message.substr(8); // "^AACTION "
				if (message[message.size() - 1] == (char)1)
					message = message.substr(0, message.size() - 1);
				if (isChanType(parsedLine[1][0], server)) {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end() && message != ""; ++modIter)
						message = modIter->second->onOutChannelAction(server, parsedLine[1], '0', message);
					if (message == "")
						return "";
					std::ostringstream result;
					result << (char)1 << "ACTION " << message << (char)1;
					return result.str();
				} else if (isChanType(parsedLine[1][1], server)) {
					char status = parsedLine[1][0];
					std::string channel = parsedLine[1].substr(1);
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end() && message != ""; ++modIter)
						message = modIter->second->onOutChannelAction(server, channel, status, message);
					if (message == "")
						return "";
					std::ostringstream result;
					result << (char)1 << "ACTION " << message << (char)1;
					return result.str();
				} else {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end() && message != ""; ++modIter)
						message = modIter->second->onOutUserAction(server, parsedLine[1], message);
					if (message == "")
						return "";
					std::ostringstream result;
					result << (char)1 << "ACTION " << message << (char)1;
					return result.str();
				}
			} else {
				message = message.substr(1);
				if (message[message.size() - 1] == (char)1)
					message = message.substr(0, message.size() - 1);
				if (isChanType(parsedLine[1][0], server)) {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end() && message != ""; ++modIter)
						message = modIter->second->onOutChannelCTCP(server, parsedLine[1], '0', message);
					if (message == "")
						return "";
					std::ostringstream result;
					result << (char)1 << message << (char)1;
					return result.str();
				} else if (isChanType(parsedLine[1][1], server)) {
					char status = parsedLine[1][0];
					std::string channel = parsedLine[1].substr(1);
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end() && message != ""; ++modIter)
						message = modIter->second->onOutChannelCTCP(server, channel, status, message);
					if (message == "")
						return "";
					std::ostringstream result;
					result << (char)1 << message << (char)1;
					return result.str();
				} else {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end() && message != ""; ++modIter)
						message = modIter->second->onOutUserCTCP(server, parsedLine[1], message);
					if (message == "")
						return "";
					std::ostringstream result;
					result << (char)1 << message << (char)1;
					return result.str();
				}
			}
		} else {
			if (isChanType(parsedLine[1][0], server)) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end() && message != ""; ++modIter)
					message = modIter->second->onOutChannelMessage(server, parsedLine[1], '0', message);
				return message;
			} else if (isChanType(parsedLine[1][1], server)) {
				char status = parsedLine[1][0];
				std::string channel = parsedLine[1].substr(1);
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end() && message != ""; ++modIter)
					message = modIter->second->onOutChannelMessage(server, channel, status, message);
				return message;
			} else {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end() && message != ""; ++modIter)
					message = modIter->second->onOutUserMessage(server, parsedLine[1], parsedLine[2]);
				return message;
			}
		}
	} else if (parsedLine[0] == "NOTICE") {
		if (message[0] == (char)1) {
			message = message.substr(1);
			if (message[message.size() - 1] == (char)1)
				message = message.substr(0, message.size() - 1);
			if (isChanType(parsedLine[1][0], server)) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end() && message != ""; ++modIter)
					message = modIter->second->onOutChannelCTCPReply(server, parsedLine[1], '0', message);
				if (message == "")
					return "";
				std::ostringstream result;
				result << (char)1 << message << (char)1;
				return result.str();
			} else if (isChanType(parsedLine[1][1], server)) {
				char status = parsedLine[1][0];
				std::string channel = parsedLine[1].substr(1);
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end() && message != ""; ++modIter)
					message = modIter->second->onOutChannelCTCPReply(server, channel, status, message);
				if (message == "")
					return "";
				std::ostringstream result;
				result << (char)1 << message << (char)1;
				return result.str();
			} else {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end() && message != ""; ++modIter)
					message = modIter->second->onOutUserCTCPReply(server, parsedLine[1], message);
				if (message == "")
					return "";
				std::ostringstream result;
				result << (char)1 << message << (char)1;
				return result.str();
			}
		} else {
			if (isChanType(parsedLine[1][0], server)) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end() && message != ""; ++modIter)
					message = modIter->second->onOutChannelNotice(server, parsedLine[1], '0', message);
				return message;
			} else if (isChanType(parsedLine[1][1], server)) {
				char status = parsedLine[1][0];
				std::string channel = parsedLine[1].substr(1);
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end() && message != ""; ++modIter)
					message = modIter->second->onOutChannelNotice(server, channel, status, message);
				return message;
			} else {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end() && message != ""; ++modIter)
					message = modIter->second->onOutUserNotice(server, parsedLine[1], parsedLine[2]);
				return message;
			}
		}
	}
	return "";
}

void Base::callModulesHookSend(std::string server, std::vector<std::string> parsedLine, std::tr1::unordered_map<std::string, Module*>* modules) {
	if (parsedLine[0] == "PRIVMSG") {
		if (parsedLine[2][0] == (char)1) {
			parsedLine[2] = parsedLine[2].substr(1);
			if (parsedLine[2][parsedLine[2].size() - 1] == (char)1)
				parsedLine[2] = parsedLine[2].substr(0, parsedLine[2].size() - 1);
			if (parsedLine[2].substr(0, parsedLine[2].find_first_of(' ')) == "ACTION") {
				std::string message = parsedLine[2].substr(7);
				if (isChanType(parsedLine[1][0], server)) {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
						modIter->second->onSendChannelAction(server, parsedLine[1], '0', message);
				} else if (isChanType(parsedLine[1][1], server)) {
					char status = parsedLine[1][0];
					std::string channel = parsedLine[1].substr(1);
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
						modIter->second->onSendChannelAction(server, channel, status, message);
				} else {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
						modIter->second->onSendUserAction(server, parsedLine[1], message);
				}
			} else {
				if (isChanType(parsedLine[1][0], server)) {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
						modIter->second->onSendChannelCTCP(server, parsedLine[1], '0', parsedLine[2]);
				} else if (isChanType(parsedLine[1][1], server)) {
					char status = parsedLine[1][0];
					std::string channel = parsedLine[1].substr(1);
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
						modIter->second->onSendChannelCTCP(server, channel, status, parsedLine[2]);
				} else {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
						modIter->second->onSendUserCTCP(server, parsedLine[1], parsedLine[2]);
				}
			}
		} else {
			if (isChanType(parsedLine[1][0], server)) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
					modIter->second->onSendChannelMessage(server, parsedLine[1], '0', parsedLine[2]);
			} else if (isChanType(parsedLine[1][1], server)) {
				char status = parsedLine[1][0];
				std::string channel = parsedLine[1].substr(1);
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
					modIter->second->onSendChannelMessage(server, channel, status, parsedLine[2]);
			} else {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
					modIter->second->onSendUserMessage(server, parsedLine[1], parsedLine[2]);
			}
		}
	} else if (parsedLine[0] == "NOTICE") {
		if (parsedLine[2][0] == (char)1) {
			parsedLine[2] = parsedLine[2].substr(1);
			if (parsedLine[2][parsedLine[2].size() - 1] == (char)1)
				parsedLine[2] = parsedLine[2].substr(0, parsedLine[2].size() - 1);
			if (isChanType(parsedLine[1][0], server)) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
					modIter->second->onSendChannelCTCPReply(server, parsedLine[1], '0', parsedLine[2]);
			} else if (isChanType(parsedLine[1][1], server)) {
				char status = parsedLine[1][0];
				std::string channel = parsedLine[1].substr(1);
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
					modIter->second->onSendChannelCTCPReply(server, channel, status, parsedLine[2]);
			} else {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
					modIter->second->onSendUserCTCPReply(server, parsedLine[1], parsedLine[2]);
			}
		} else {
			if (isChanType(parsedLine[1][0], server)) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
					modIter->second->onSendChannelNotice(server, parsedLine[1], '0', parsedLine[2]);
			} else if (isChanType(parsedLine[1][1], server)) {
				char status = parsedLine[1][0];
				std::string channel = parsedLine[1].substr(1);
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
					modIter->second->onSendChannelNotice(server, channel, status, parsedLine[2]);
			} else {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
					modIter->second->onSendUserNotice(server, parsedLine[1], parsedLine[2]);
			}
		}
	}
}

void Base::callPreConnectModulesHook(std::string server, std::tr1::unordered_map<std::string, Module*>* modules) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
		modIter->second->onPreConnect(server);
}

void Base::callConnectModulesHook(std::string server, std::tr1::unordered_map<std::string, Module*>* modules) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
		modIter->second->onConnect(server);
}

void Base::callQuitModulesHook(std::string server, std::tr1::unordered_map<std::string, Module*>* modules) {
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules->begin(); modIter != modules->end(); ++modIter)
		modIter->second->onQuit(server);
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
		modules.insert(modIter);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumHighModules.begin(); modIter != mediumHighModules.end(); ++modIter)
		modules.insert(modIter);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = normalModules.begin(); modIter != normalModules.end(); ++modIter)
		modules.insert(modIter);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = mediumLowModules.begin(); modIter != mediumLowModules.end(); ++modIter)
		modules.insert(modIter);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = lowModules.begin(); modIter != lowModules.end(); ++modIter)
		modules.insert(modIter);
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

std::string Base::userIdent(std::string server, std::string channel, std::string user) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return "";
	return servIter->second->userIdent(channel, user);
}

std::string Base::userHost(std::string server, std::string channel, std::string user) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return "";
	return servIter->second->userHost(channel, user);
}

std::pair<char, char> Base::userStatus(std::string server, std::string channel, std::string user) {
	std::tr1::unordered_map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::pair<char, char> ('0', ' '); // pair for normal user
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
		protoFile = dlopen(fileLoc.c_str, RTLD_NOW);
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
	
	Module* newModule = (Module*) spawnModule(modConf->second, this, modName, debugLevel);
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
	else if (lowModules.find(moduleToUnload[0] != lowModules.end())
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