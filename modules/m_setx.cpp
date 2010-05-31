#include "modinclude.h"

class SetX : public Module {
	public:
		int botAPIversion();
		void onLoadComplete();
		void onRehash();
		void onNumeric(std::string server, std::string numeric, std::vector<std::string> parsedLine);
	private:
		std::vector<std::string> xServers;
};

int SetX::botAPIversion() {
	return 1001;
}

void SetX::onLoadComplete() {
	std::string serverList = config["servers"];
	while (serverList != "") {
		std::string aServer = serverList.substr(0, serverList.find_first_of(','));
		xServers.push_back(aServer);
		if (serverList.find_first_of(',') == std::string::npos)
			return;
		serverList.substr(serverList.find_first_of(',') + 1);
	}
}

void SetX::onRehash() {
	xServers.clear();
	onLoadComplete();
}

void SetX::onNumeric(std::string server, std::string numeric, std::vector<std::string> parsedLine) {
	if (numeric != "001")
		return;
	for (unsigned int i = 0; i < xServers.size(); i++) {
		if (xServers[i] == server) {
			setMode(server, getServerData(server)["nick"], 'x', true);
			return;
		}
	}
}

extern "C" Module* spawn() {
	return new SetX;
}