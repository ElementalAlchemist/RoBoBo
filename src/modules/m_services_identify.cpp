#include "modinclude.h"

class ServicesID : public Module {
	public:
		ServicesID(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug);
		int botAPIversion();
		void onNumeric(std::string server, std::string client, std::string numeric, std::vector<std::string> parsedLine);
		std::string description();
};

ServicesID::ServicesID(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, short unsigned int debug): Module(modConf, modFace, modName, dir, debug) {}

int ServicesID::botAPIversion() {
	return 2001;
}

void ServicesID::onNumeric(std::string server, std::string client, std::string numeric, std::vector<std::string> parsedLine) {
	if (numeric == "001" && config[server+"/password"] != "")
		sendPrivMsg(server, client, config[server+"/service"], config[server+"/command"] + " " + (config[server+"/nickname"] != "" ? config[server+"/nickname"] + " " : "") + config[server+"/password"]);
}

std::string ServicesID::description() {
	return "Allows identification/authentification to services.";
}

MODULE_SPAWN(ServicesID)