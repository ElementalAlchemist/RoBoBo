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
		std::string getDesc();
	private:
		void dccConnect(std::string server, std::string nick, std::string ip, std::string port);
		void dccListen(std::string id, Socket* listenSocket);
		static void* dccListen_thread(void* args);
		std::vector<pthread_t> threads;
		std::tr1::unordered_map<std::string, Socket*> activeConnections;
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
	dccListenArg listenData;
	listenData.modPtr = this;
	listenData.sockPtr = dccSocket;
	pthread_t newThread;
	threads.push_back(newThread);
	pthread_create(&threads[threads.size()-1], NULL, &dccListen_thread, (void*)&listenData);
}

void* m_dccchat::dccListen_thread(void* args) {
	dccListenArg* listenData = (dccListenArg*) args;
	listenData->modPtr->dccListen(listenData->id, listenData->sockPtr);
	return NULL;
}

void m_dccchat::dccListen(std::string id, Socket* listenSocket) {
	std::vector<std::string> reportingModules;
	while (true) {
		if (!listenSocket->isConnected())
			break;
		std::string receivedMsg = listenSocket->receive();
		std::tr1::unordered_map<std::string, Module*> modules = getModules(); // get a new one each time in case it is updated
		for (std::tr1::unordered_map<std::string, std::string>::iterator hookIter = moduleTriggers.begin(); hookIter != moduleTriggers.end(); ++hookIter) {
			if (hookIter->first == receivedMsg.substr(0, receivedMsg.find_first_of(' ')))
				reportingModules.push_back(hookIter->second);
		}
		for (unsigned int i = 0; i < reportingModules.size(); i++) {
			std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.find(reportingModules[i]);
			if (modIter == modules.end())
				reportingModules.erase(reportingModules.begin()+i);
			else {
				dccChat* dccMod = (dccChat*)modIter->second;
				dccMod->onDCCReceive(id, receivedMsg);
			}
		}
	}
	delete listenSocket;
	activeConnections.erase(id);
}