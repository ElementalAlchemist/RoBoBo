#include "modinclude.h"

class SetX : public Module {
	public:
		SetX(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug);
		int botAPIversion();
		bool onLoadComplete();
		void onRehash();
		void onNumeric(std::string server, std::string client, std::string numeric, std::vector<std::string> parsedLine);
		std::string description();
	private:
		std::vector<std::string> xServers;
};

SetX::SetX(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug) : Module(modConf, modFace, modName, dir, debug) {}

int SetX::botAPIversion() {
	return 2001;
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

void SetX::onNumeric(std::string server, std::string client, std::string numeric, std::vector<std::string> parsedLine) {
	if (!serverIsClient(server))
		return;
	if (numeric != "001")
		return;
	for (unsigned int i = 0; i < xServers.size(); i++) {
		if (xServers[i] == server) {
			std::list<std::string> addMode;
			addMode.push_back("cloak");
			setMode(server, client, clientInfo(server, client)["nick"], addMode, std::list<std::string> ());
			return;
		}
	}
}

std::string SetX::description() {
	return "Allows the automatic setting of usermode x on servers.";
}

MODULE_SPAWN(SetX)