#include "modinclude.h"
#include "dcc_chat.h"
#define DCC_CHAT
#include "s_plaintext.cpp" // steal use of existing socket system
#include <pthread.h>

class DCCChatModule;
struct dccListenArg {
	DCCChatModule* modPtr;
	std::string id;
	Socket* sockPtr;
};

class DCCChatModule : public dccSender {
	public:
		DCCChatModule(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug);
		int botAPIversion();
		void onNickChangePre(std::string server, std::string oldNick, std::string newNick);
		bool onUserCTCP(std::string server, std::string client, std::string nick, std::string message);
		std::vector<std::string> getConnections();
		void dccSend(std::string recipient, std::string message);
		std::vector<std::string> abilities();
		bool hookDCCMessage(std::string modName, std::string hookMsg);
		void unhookDCCSession(std::string modName, std::string dccid);
		void closeDCCConnection(std::string dccid);
		std::string description();
	private:
		void dccConnect(std::string server, std::string nick, std::string ip, std::string port);
		void dccListen(std::string id, Socket* listenSocket);
		static void* dccListen_thread(void* args);
		std::vector<pthread_t*> threads;
		std::tr1::unordered_map<std::string, Socket*> activeConnections;
		std::tr1::unordered_map<std::string, std::vector<std::string> > reportingModules;
		std::tr1::unordered_map<std::string, std::string> moduleTriggers;
};

DCCChatModule::DCCChatModule(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug) : dccSender(modConf, modFace, modName, dir, debug) {}

int DCCChatModule::botAPIversion() {
	return 2000;
}

void DCCChatModule::onNickChangePre(std::string server, std::string oldNick, std::string newNick) {
	std::tr1::unordered_map<std::string, Socket*>::iterator dccIter = activeConnections.find(server + "/" + oldNick);
	if (dccIter != activeConnections.end()) {
		Socket* thisSocket = dccIter->second;
		activeConnections.erase(dccIter);
		activeConnections.insert(std::pair<std::string, Socket*> (server + "/" + newNick, thisSocket));
	}
}

bool DCCChatModule::onUserCTCP(std::string server, std::string client, std::string nick, std::string message) {
	std::vector<std::string> messageParts = splitBySpace(message);
	if (messageParts[0] == "DCC" && messageParts[1] == "CHAT" && messageParts[2] == "chat") {
		if (activeConnections.find(server + "/" + nick) == activeConnections.end())
			dccConnect(server, nick, messageParts[3], messageParts[4]);
		else
			sendNotice(server, client, nick, "You already have an active DCC chat session!");
	}
	return true;
}

std::vector<std::string> DCCChatModule::getConnections() {
	std::vector<std::string> connections;
	for (std::tr1::unordered_map<std::string, Socket*>::iterator connIter = activeConnections.begin(); connIter != activeConnections.end(); ++connIter)
		connections.push_back(connIter->first);
	return connections;
}

void DCCChatModule::dccSend(std::string recipient, std::string message) {
	std::tr1::unordered_map<std::string, Socket*>::iterator dccIter = activeConnections.find(recipient);
	if (dccIter == activeConnections.end())
		return;
	dccIter->second->sendData(message);
}

std::vector<std::string> DCCChatModule::abilities() {
	std::vector<std::string> abilities;
	abilities.push_back("DCC_CHAT");
	return abilities;
}

bool DCCChatModule::hookDCCMessage(std::string modName, std::string hookMsg) {
	if (moduleTriggers.find(hookMsg) == moduleTriggers.end()) {
		moduleTriggers.insert(std::pair<std::string, std::string> (hookMsg, modName));
		return true;
	}
	return false;
}

void DCCChatModule::unhookDCCSession(std::string modName, std::string dccid) {
	std::tr1::unordered_map<std::string, std::vector<std::string> >::iterator reportingModIter = reportingModules.find(dccid);
	if (reportingModIter == reportingModules.end())
		return;
	for (unsigned int i = 0; i < reportingModIter->second.size(); i++) {
		if (reportingModIter->second[i] == modName)
			reportingModIter->second.erase(reportingModIter->second.begin()+i);
	}
}

std::string DCCChatModule::description() {
	return "This module gives the bot DCC CHAT support, allowing users to enter in DCC CHAT communication with the bot.";
}

void DCCChatModule::dccConnect(std::string server, std::string nick, std::string ip, std::string port) {
	Socket* dccSocket = new PlainText();
	std::istringstream portNumber (port);
	unsigned short dccPort;
	portNumber >> dccPort;
	dccSocket->connectServer(ip, dccPort);
	activeConnections.insert(std::pair<std::string, Socket*> (server + "/" + nick, dccSocket));
	reportingModules.insert(std::pair<std::string, std::vector<std::string> > (server + "/" + nick, std::vector<std::string> ()));
	dccListenArg listenData;
	listenData.modPtr = this;
	listenData.id = server + "/" + nick;
	listenData.sockPtr = dccSocket;
	pthread_t newThread;
	threads.push_back(&newThread);
	void* listenArgs = (void*) &listenData;
	pthread_create(&newThread, NULL, &dccListen_thread, listenArgs);
	sleep(1); // make the arguments passed to the thread not die before the thread function can read them.
}

void* DCCChatModule::dccListen_thread(void* args) {
	dccListenArg* listenData = (dccListenArg*) args;
	listenData->modPtr->dccListen(listenData->id, listenData->sockPtr);
	return NULL;
}

void DCCChatModule::dccListen(std::string id, Socket* listenSocket) {
	std::tr1::unordered_map<std::string, std::vector<std::string> >::iterator ourReportingModules = reportingModules.find(id);
	while (true) {
		if (!listenSocket->isConnected())
			break;
		std::string receivedMsg = listenSocket->receive();
		if (receivedMsg == "")
			break;
		if (debugLevel >= 3)
			std::cout << "DCC " << id << ":" << receivedMsg << std::endl;
		if (receivedMsg == "closedcc") {
			dccSend(id, "Disconnecting DCC...");
			break;
		}
		std::tr1::unordered_map<std::string, Module*> moduleList = modules(); // get a new one each time in case it is updated
		for (std::tr1::unordered_map<std::string, std::string>::iterator hookIter = moduleTriggers.begin(); hookIter != moduleTriggers.end(); ++hookIter) {
			if (hookIter->first == receivedMsg.substr(0, receivedMsg.find_first_of(' '))) {
				bool alreadyReporting = false;
				for (unsigned int i = 0; i < ourReportingModules->second.size(); i++) {
					if (ourReportingModules->second[i] == hookIter->second) {
						alreadyReporting = true;
						break;
					}
				}
				if (!alreadyReporting)
					ourReportingModules->second.push_back(hookIter->second);
			}
		}
		for (unsigned int i = 0; i < ourReportingModules->second.size(); i++) {
			std::tr1::unordered_map<std::string, Module*>::iterator modIter = moduleList.find(ourReportingModules->second[i]);
			if (modIter == moduleList.end())
				ourReportingModules->second.erase(ourReportingModules->second.begin()+i);
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
	std::tr1::unordered_map<std::string, Module*> moduleList = modules();
	for (unsigned int i = 0; i < ourReportingModules->second.size(); i++) {
		std::tr1::unordered_map<std::string, Module*>::iterator modIter = moduleList.find(ourReportingModules->second[i]);
		dccChat* dccMod = (dccChat*) modIter->second;
		dccMod->onDCCEnd(id); // call the DCC end hook for each watching module as the DCC session ends
	}
	listenSocket->closeConnection();
	delete listenSocket;
	activeConnections.erase(id);
}

void DCCChatModule::closeDCCConnection(std::string dccid) {
	std::tr1::unordered_map<std::string, Socket*>::iterator dccConnection = activeConnections.find(dccid);
	if (dccConnection == activeConnections.end())
		return;
	dccConnection->second->closeConnection();
}

MODULE_SPAWN(DCCChatModule)