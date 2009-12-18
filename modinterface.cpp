#include "modinterface.h"

#ifndef MODIFACE_ROBOBO
#define MODIFACE_ROBOBO
class ModuleInterface {
	public:
		ModuleInterface(std::tr1::unordered_map<std::string, Server>* serverMap, std::tr1::unordered_map<std::string, Module>* moduleMap);
		void sendToServer(std::string server, std::string rawLine);
		void callHook(std::string server, std::vector<std::string> parsedLine);
		void callHookOut(std::string server, std::vector<std::string> parsedLine);
		void callHookSelf(std::string server, std::vector<std::string> parsedLine);
	private:
		std::tr1::unordered_map<std::string, Server>* servers;
		std::tr1::unordered_map<std::string, Module>* modules;
		std::string parseNickFromHost(std::string host);
		bool charIsNumeric(char number);
		bool isChanType(char chanPrefix);
};

ModuleInterface::ModuleInterface(std::tr1::unordered_map<std::string, Server>* serverMap, std::tr1::unordered_map<std::string, Module>* moduleMap) {
	servers = serverMap;
	modules = moduleMap;
}

void ModuleInterface::sendToServer(std::string server, std::string rawLine) {
	for (std::tr1::unordered_map<std::string, Server>::iterator serverIter = servers->begin(); serverIter != servers->end(); serverIter++) {
		if (serverIter->first == server)
			serverIter->second.sendLine(rawLine);
	}
}

void ModuleInterface::callHook(std::string server, std::vector<std::string> parsedLine) {
	if (parsedLine[1] == "PRIVMSG") {
		if (parsedLine[3][0] == (char)1) {
			if (parsedLine[3][parsedLine.size()-1] == (char)1)
				parsedLine[3] = parsedLine[3].substr(1, parsedLine[3].size()-2);
			else
				parsedLine[3] = parsedLine[3].substr(1);
			if (isChanType(parsedLine[2][0])) {
				for (std::tr1::unordered_map<std::string, Module>::iterator modIter = modules->begin(); modIter != modules->end(); modIter++)
					modIter->second.onChannelCTCP(server, parsedLine[2], '0', parseNickFromHost(parsedLine[0]), parsedLine[3]);
			} else if (isChanType(parsedLine[2][1]) {
				for (std::tr1::unordered_map<std::string, Module>::iterator modIter = modules->begin(); modIter != modules->end(); modIter++)
					modIter->second.onChannelCTCP(server, parsedLine[2].substr(1), parsedLine[2][0], parseNickFromHost(parsedLine[0]), parsedLine[3]);
			} else {
				for (std::tr1::unordered_map<std::string, Module>::iterator modIter = modules->begin(); modIter != modules->end(); modIter++)
					modIter->second.onUserCTCP(server, parseNickFromHost(parsedLine[0]), parsedLine[3]);
			}
		} else {
			if (isChanType(parsedLine[2][0])) {
				for (std::tr1::unordered_map<std::string, Module>::iterator modIter = modules->begin(); modIter != modules->end(); modIter++)
					modIter->second.onChannelMsg(server, parsedLine[2], '0', parseNickFromHost(parsedLine[0]), parsedLine[3]);
			} else if (isChanType(parsedLine[2][1])) {
				for (std::tr1::unordered_map<std::string, Module>::iterator modIter = modules->begin(); modIter != modules->end(); modIter++)
					modIter->second.onChannelMsg(server, parsedLine[2].substr(1), parsedLine[2][0], parseNickFromHost(parsedLine[0]), parsedLine[3]);
			} else {
				for (std::tr1::unordered_map<std::string, Module>::iterator modIter = modules->begin(); modIter != modules->end(); modIter++)
					modIter->second.onUserMsg(server, parseNickFromHost(parsedLine[0]), parsedLine[3]);
			}
		}
	} else if (parsedLine[1] == "NOTICE") {
		if (parsedLine[3][0] == (char)1) {
			if (parsedLine[3][parsedLine.size()-1] == (char)1)
				parsedLine[3] = parsedLine[3].substr(1, parsedLine[3].size()-2);
			else
				parsedLine[3] = parsedLine[3].substr(1);
			if (isChanType(parsedLine[2][0])) {
				for (std::tr1::unordered_map<std::string, Module>::iterator modIter = modules->begin(); modIter != modules->end(); modIter++)
					modIter->second.onChannelCTCPReply(server, parsedLine[2], '0', parseNickFromHost(parsedLine[0]), parsedLine[3]);
			} else if (isChanType(parsedLine[2][1])) {
				for (std::tr1::unordered_map<std::string, Module>::iterator modIter = modules->begin(); modIter != modules->end(); modIter++)
					modIter->second.onChannelCTCPReply(server, parsedLine[2].substr(1), parsedLine[2][0], parseNickFromHost(parsedLine[0]), parsedLine[3]);
			} else {
				for (std::tr1::unordered_map<std::string, Module>::iterator modIter = modules->begin(); modIter != modules->end(); modIter++)
					modIter->second.onUserCTCPReply(server, parseNickFromHost(parsedLine[0]), parsedLine[3]);
			}
		} else {
			if (isChanType(parsedLine[2][0])) {
				for (std::tr1::unordered_map<std::string, Module>::iterator modIter = modules->begin(); modIter != modules->end(); modIter++)
					modIter->second.onChannelNotice(server, parsedLine[2], '0', parseNickFromHost(parsedLine[0]), parsedLine[3]);
			} else if (isChanType(parsedLine[2][1])) {
				for (std::tr1::unordered_map<std::string, Module>::iterator modIter = modules->begin(); modIter != modules->end(); modIter++)
					modIter->second.onChannelNotice(server, parsedLine[2].substr(1), parsedLine[2][0], parseNickFromHost(parsedLine[0]), parsedLine[3]);
			} else {
				for (std::tr1::unordered_map<std::string, Module>::iterator modIter = modules->begin(); modIter != modules->end(); modIter++)
					modIter->second.onUserNotice(server, parseNickFromHost(parsedLine[0]), parsedLine[3]);
			}
		}
	} else if (parsedLine[1] == "JOIN") {
		if (parsedLine[0][0] == ':')
			parsedLine[0] = parsedLine[0].substr(1);
		for (std::tr1::unordered_map<std::string, Module>::iterator modIter = modules->begin(); modIter != modules->end(); modIter++)
			modIter->second.onChannelJoin(server, parsedLine[2], parsedLine[0]);
	} else if (parsedLine[1] == "PART") {
		if (parsedLine[0][0] == ':')
			parsedLine[0] = parsedLine[0].substr(1);
		for (std::tr1::unordered_map<std::string, Module>::iterator modIter = modules->begin(); modIter != modules->end(); modIter++)
			modIter->second.onChannelPart(server, parsedLine[2], parsedLine[0], parsedLine[3]);
	} else if (parsedLine[1] == "QUIT") {
		if (parsedLine[0][0] == ':')
			parsedLine[0] = parsedLine[0].substr(1);
		for (std::tr1::unordered_map<std::string, Module>::iterator modIter = modules->begin(); modIter != modules->end(); modIter++)
			modIter->second.onUserQuit(server, parsedLine[0], parsedLine[2]);
	}
}

void ModuleInterface::callHookOut(std::string server, std::vector<std::string> parsedLine) {
	
}

void ModuleInterface::callHookSelf(std::string server, std::vector<std::string> parsedLine) {
	// copypasta callHook, changing the hook names for Self
}

std::string ModuleInterface::parseNickFromHost(std::string host) {
	if (host[0] == ':')
		host = host.substr(1);
	return host.substr(0, host.find_first_of('!'));
}

bool ModuleInterface::charIsNumeric(char number) {
	if (number == '0' || number == '1' || number == '2' || number == '3' || number == '4' || number == '5' || number == '6' || number == '7' || number == '8' || number == '9')
		return true;
	return false;
}

bool ModuleInterface::isChanType(char chanPrefix) {
	std::vector<char> prefixes;
	for (std::tr1::unordered_map<std::string, Server>::iterator serverIter = servers->begin(); serverIter != servers->end(); serverIter++) {
		prefixes = serverIter->second.getChanTypes();
		for (unsigned int i = 0; i < prefixes.size(); i++) {
			if (chanPrefix == prefixes[i])
				return true;
		}
	}
	return false;
}
#endif