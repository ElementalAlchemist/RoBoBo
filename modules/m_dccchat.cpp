#include "modinclude.h"
#include "dcc_chat.h"
#include <pthread.h>

class m_dccchat;
struct dccListenArg {
	m_dccchat* modPtr;
	std::string id;
	Socket* sockPtr;
};

class m_dccchat : public dccSender {
	public:
		void onUserCTCP(std::string server, std::string nick, std::string message);
		std::vector<std::string> getConnections();
		void sendDCCMessage(std::string recipient, std::string message);
		std::vector<std::string> getAbilities();
		bool hookDCCMessage(std::string modName, std::string hookMsg);
		void unhookDCCSession(std::string modName, std::string dccid);
		std::string getDesc();
	private:
		void dccConnect(std::string server, std::string nick, std::string ip, std::string port);
		void dccListen(std::string id, Socket* listenSocket);
		static void* dccListen_thread(void* args);
		std::vector<pthread_t*> threads;
		std::tr1::unordered_map<std::string, Socket*> activeConnections;
		std::tr1::unordered_map<std::string, std::vector<std::string> > reportingModules;
		std::tr1::unordered_map<std::string, std::string> moduleTriggers;
};

void m_dccchat::onUserCTCP(std::string server, std::string nick, std::string message) {
	std::vector<std::string> messageParts = splitBySpace(message);
	if (messageParts[0] == "DCC" && messageParts[1] == "CHAT") {
		if (activeConnections.find(server + "/" + nick) == activeConnections.end())
			dccConnect(server, nick, messageParts[3], messageParts[4]);
		else
			sendNotice(server, nick, "You already have an active DCC chat session!");
	}
}

std::vector<std::string> m_dccchat::getConnections() {
	std::vector<std::string> connections;
	for (std::tr1::unordered_map<std::string, Socket*>::iterator connIter = activeConnections.begin(); connIter != activeConnections.end(); ++connIter)
		connections.push_back(connIter->first);
	return connections;
}

void m_dccchat::sendDCCMessage(std::string recipient, std::string message) {
	std::tr1::unordered_map<std::string, Socket*>::iterator dccIter = activeConnections.find(recipient);
	if (dccIter == activeConnections.end())
		return;
	dccIter->second->sendData(message);
}

std::vector<std::string> m_dccchat::getAbilities() {
	std::vector<std::string> abilities;
	abilities.push_back("DCC_CHAT");
	return abilities;
}

bool m_dccchat::hookDCCMessage(std::string modName, std::string hookMsg) {
	if (moduleTriggers.find(hookMsg) == moduleTriggers.end()) {
		moduleTriggers.insert(std::pair<std::string, std::string> (hookMsg, modName));
		return true;
	}
	return false;
}

void m_dccchat::unhookDCCSession(std::string modName, std::string dccid) {
	std::tr1::unordered_map<std::string, std::vector<std::string> >::iterator reportingModIter = reportingModules.find(dccid);
	if (reportingModIter == reportingModules.end())
		return;
	for (unsigned int i = 0; i < reportingModIter->second.size(); i++) {
		if (reportingModIter->second[i] == modName)
			reportingModIter->second.erase(reportingModIter->second.begin()+i);
	}
}

std::string m_dccchat::getDesc() {
	return "This module gives the bot DCC CHAT support, allowing users to enter in DCC CHAT communication with the bot.";
}

void m_dccchat::dccConnect(std::string server, std::string nick, std::string ip, std::string port) {
	Socket* dccSocket = new Socket();
	std::istringstream portNumber (port);
	unsigned short dccPort;
	portNumber >> dccPort;
	dccSocket->connectServer(ip, dccPort);
	activeConnections.insert(std::pair<std::string, Socket*> (server + "/" + nick, dccSocket));
	reportingModules.insert(std::pair<std::string, std::vector<std::string> > (server + "/" + nick, std::vector<std::string> ()));
	dccListenArg listenData;
	listenData.modPtr = this;
	listenData.sockPtr = dccSocket;
	pthread_t fullNewThread;
	pthread_t* newThread = &fullNewThread;
	threads.push_back(newThread);
	pthread_create(newThread, NULL, &dccListen_thread, (void*)&listenData);
}

void* m_dccchat::dccListen_thread(void* args) {
	dccListenArg* listenData = (dccListenArg*) args;
	listenData->modPtr->dccListen(listenData->id, listenData->sockPtr);
	return NULL;
}

void m_dccchat::dccListen(std::string id, Socket* listenSocket) {
	std::vector<std::string> ourReportingModules; = reportingModules.find(id)->second;
	while (true) {
		if (!listenSocket->isConnected())
			break;
		std::string receivedMsg = listenSocket->receive();
		std::tr1::unordered_map<std::string, Module*> modules = getModules(); // get a new one each time in case it is updated
		for (std::tr1::unordered_map<std::string, std::string>::iterator hookIter = moduleTriggers.begin(); hookIter != moduleTriggers.end(); ++hookIter) {
			if (hookIter->first == receivedMsg.substr(0, receivedMsg.find_first_of(' ')))
				ourReportingModules.push_back(hookIter->second);
		}
		for (unsigned int i = 0; i < ourReportingModules.size(); i++) {
			std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.find(ourReportingModules[i]);
			if (modIter == modules.end())
				ourReportingModules.erase(ourReportingModules.begin()+i);
			else {
				std::vector<std::string> modSupports = modIter->second->supports();
				for (unsigned int i = 0; i < modSupports.size(); i++) {
					if (modSupports[i] == "DCC_CHAT") {
						dccChat* dccMod = (dccChat*)modIter->second;
						dccMod->onDCCReceive(id, receivedMsg);
						break;
					}
				}
			}
		}
	}
	std::tr1::unordered_map<std::string, Module*> modules = getModules();
	for (unsigned int i = 0; i < reportingModules.size(); i++) {
		std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.find(ourReportingModules[i]);
		dccChat* dccMod = (dccChat*) modIter->second;
		dccMod->onDCCEnd(id); // call the DCC end hook for each watching module as the DCC session ends
	}
	delete listenSocket;
	activeConnections.erase(id);
}

extern "C" Module* spawn() {
	return new m_dccchat;
}