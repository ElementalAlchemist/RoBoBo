#include "modinclude.h"

class SetX : public Module {
	public:
		int botAPIversion();
		bool onLoadComplete();
		void onRehash();
		void onNumeric(std::string server, std::string numeric, std::vector<std::string> parsedLine);
		std::string description();
	private:
		std::vector<std::string> xServers;
};

int SetX::botAPIversion() {
	return 2000;
}

bool SetX::onLoadComplete() {
	std::string serverList = config["servers"];
	while (serverList != "") {
		std::string aServer = serverList.substr(0, serverList.find_first_of(','));
		xServers.push_back(aServer);
		if (serverList.find_first_of(',') == std::string::npos)
			return true; // loading complete
		serverList.substr(serverList.find_first_of(',') + 1);
	}
	return true;
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
			setMode(server, serverData(server)["nick"], 'x', true);
			return;
		}
	}
}

std::string SetX::description() {
	return "Allows the automatic setting of usermode x on servers.";
}

extern "C" Module* spawn() {
	return new SetX;
}