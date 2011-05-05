#include "modinclude.h"
#include "cap.h"

class Cap : public CapModule {
	public:
		Cap();
		int botAPIversion();
		void onModuleChange();
		void onOtherData(std::string server, std::vector<std::string> parsedLine);
		void onConnect(std::string server);
		void capRegister(std::string capCommand, std::string moduleName);
		void blockCap(std::string server, std::string moduleName);
		void continueCap(std::string server, std::string moduleName);
	private:
		void endCap(std::string server);
		std::tr1::unordered_map<std::string, std::vector<std::string> > capCommands;
		std::tr1::unordered_map<std::string, std::vector<std::string> > blockingModules;
};

Cap::Cap() {
	std::vector<std::string> theCore;
	theCore.push_back("");
	capCommands.insert(std::pair<std::string, std::vector<std::string> > ("multi-prefix", theCore));
	capCommands.insert(std::pair<std::string, std::vector<std::string> > ("userhost-in-names", theCore));
}

int Cap::botAPIversion() {
	return 2000;
}

void Cap::onModuleChange() {
	std::tr1::unordered_map<std::string, Module*> modulesList = modules();
	for (std::tr1::unordered_map<std::string, std::vector<std::string> >::iterator capIter = capCommands.begin(); capIter != capCommands.end(); ++capIter) {
		for (unsigned int i = 0; i < capIter->second.size(); i++) {
			if (modulesList.find(capIter->second[i]) == modulesList.end()) {
				capIter->second.erase(capIter->second.begin() + i);
				i--;
			}
		}
	}
	for (std::tr1::unordered_map<std::string, std::vector<std::string> >::iterator blockIter = blockingModules.begin(); blockIter != blockingModules.end(); ++blockIter) {
		for (unsigned int i = 0; i < blockIter->second.size(); i++) {
			if (modulesList.find(blockIter->second[i]) == modulesList.end()) {
				blockIter->second.erase(blockIter->second.begin() + i);
				i--;
			}
		}
		endCap(blockIter->first);
	}
}

void Cap::onOtherData(std::string server, std::vector<std::string> parsedLine) {
	if (parsedLine[1] == "CAP") {
		if (parsedLine[3] == "LS") {
			std::string capReq = "";
			std::vector<std::string> serverCapab = splitBySpace(parsedLine[4]);
			for (unsigned int i = 0; i < serverCapab.size(); i++) {
				if (serverCapab[i][0] == '=' || serverCapab[i][0] == '~')
					serverCapab[i] = serverCapab[i].substr(1);
				std::tr1::unordered_map<std::string, std::vector<std::string> >::iterator capIter = capCommands.find(serverCapab[i]);
				if (capIter != capCommands.end())
					capReq += " " + serverCapab[i];
			}
			if (capReq == "")
				sendOtherCommand(server, "CAP", "END");
			else
				sendOtherCommand(server, "CAP", "REQ :" + capReq.substr(1));
		} else if (parsedLine[3] == "ACK") {
			std::vector<std::string> capAcknowledge = splitBySpace(parsedLine[4]);
			std::string ackReply = "";
			for (unsigned int i = 0; i < capAcknowledge.size(); i++) {
				char capModifier = ' ';
				if (capAcknowledge[i][0] == '=' || capAcknowledge[i][0] == '~') {
					capModifier = capAcknowledge[i][0];
					capAcknowledge[i] = capAcknowledge[i].substr(1);
				}
				std::tr1::unordered_map<std::string, std::vector<std::string> >::iterator capAck = capCommands.find(capAcknowledge[i]);
				if (capAck != capCommands.end()) {
					for (unsigned int j = 0; j < capAck->second.size(); j++) {
						if (capAck->second[j] == "")
							continue;
						CapClient* ackMod = (CapClient*) modules().find(capAck->second[j])->second;
						ackMod->onCapAccept(server, capAck->first);
					}
				}
				if (capModifier == '~')
					ackReply += " " + capAcknowledge[i];
			}
			if (ackReply != "")
				sendOtherCommand(server, "CAP", "ACK :" + ackReply.substr(1));
			endCap(server);
		} else if (parsedLine[3] == "NAK")
			endCap(server);
	}
}

void Cap::onConnect(std::string server) {
	if (serverIsClient(server)) {
		sendOtherCommand(server, "CAP", "LS");
		blockingModules[server].clear();
	}
}

void Cap::capRegister(std::string capCommand, std::string moduleName) {
	capCommands[capCommand].push_back(moduleName);
}

void Cap::blockCap(std::string server, std::string moduleName) {
	blockingModules[server].push_back(moduleName);
}

void Cap::continueCap(std::string server, std::string moduleName) {
	for (unsigned int i = 0; i < blockingModules[server].size(); i++) {
		if (blockingModules[server][i] == moduleName) {
			blockingModules[server].erase(blockingModules[server].begin() + i);
			i--;
		}
	}
	endCap(server);
}

void Cap::endCap(std::string server) {
	if (blockingModules[server].empty()) {
		sendOtherCommand(server, "CAP", "END");
		blockingModules[server].push_back(""); // insert dummy to force CAP END to not be sent again
	}
}

extern "C" Module* spawn(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, unsigned short debug) {
	return new Cap (modConf, modFace, modName, debug);
}