#include "modinclude.h"

class m_modetest : public Module {
	public:
		void onChannelMsg(std::string server, std::string channel, char status, std::string nick, std::string message);
		std::string getDesc();
};

void m_modetest::onChannelMsg(std::string server, std::string channel, char status, std::string nick, std::string message) {
	std::vector<std::string> splitMsg = splitBySpace(message);
	if (splitMsg[0] == "!mode") {
		unsigned int currParam = 2;
		bool addingMode = true;
		std::vector<std::vector<char> > chanModes = getServerChanModes(server);
		std::tr1::unordered_map<char, char> prefixes = getServerPrefixes(server);
		for (unsigned int i = 0; i < splitMsg[1].size(); i++) {
			if (splitMsg[1][i] == '+')
				addingMode = true;
			else if (splitMsg[1][i] == '-')
				addingMode = false;
			else {
				bool found = false;
				int category;
				for (std::tr1::unordered_map<char, char>::iterator prefixIter = prefixes.begin(); prefixIter != prefixes.end(); prefixIter++) {
					if (prefixIter->first == splitMsg[1][i]) {
						found = true;
						category = 0;
						break;
					}
				}
				if (!found) {
					for (unsigned int j = 0; j < chanModes.size(); j++) {
						for (unsigned int k = 0; k < chanModes[j].size(); k++) {
							if (chanModes[j][k] == splitMsg[1][i]) {
								found = true;
								category = j;
								break;
							}
						}
						if (found)
							break;
					}
				}
				if (!found)
					category = 4;
				
				if (category == 0 || category == 1 || (category == 2 && addingMode))
					setMode(server, channel, splitMsg[1][i], addingMode, splitMsg[currParam++]);
				else
					setMode(server, channel, splitMsg[1][i], addingMode);
			}
		}
	}
}

std::string m_modetest::getDesc() { return "This module sets channel modes based on the parameters given with the !mode fantasy command."; }

extern "C" Module* spawn() {
	return new m_modetest;
}

extern "C" void unspawn(Module* mod) {
	delete mod;
}