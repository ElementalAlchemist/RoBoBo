#include "modinclude.h"

class ServicesID : public Module {
	public:
		int botAPIversion();
		void onNumeric(std::string server, std::string numeric, std::vector<std::string> parsedLine);
		std::string description();
};

int ServicesID::botAPIversion() {
	return 2000;
}

void ServicesID::onNumeric(std::string server, std::string numeric, std::vector<std::string> parsedLine) {
	if (numeric == "001" && config[server+"/password"] != "")
		sendPrivMsg(server, config[server+"/service"], config[server+"/command"] + " " + (config[server+"/nickname"] != "" ? config[server+"/nickname"] + " " : "") + config[server+"/password"]);
}

std::string ServicesID::description() {
	return "Allows identification/authentification to services.";
}

extern "C" Module* spawn(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug) {
	return new ServicesID (modConf, modFace, modName, dir, debug);
}