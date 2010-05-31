#include "modinclude.h"

class ServicesID : public Module {
	public:
		int botAPIversion();
		void onNumeric(std::string server, std::string numeric, std::vector<std::string> parsedLine);
		std::string getDesc();
};

int ServicesID::botAPIversion() {
	return 1001;
}

void ServicesID::onNumeric(std::string server, std::string numeric, std::vector<std::string> parsedLine) {
	if (numeric == "001" && config[server+"/password"] != "")
		sendPrivMsg(server, config[server+"/service"], config[server+"/command"] + " " + (config[server+"/nickname"] != "" ? config[server+"/nickname"] + " " : "") + config[server+"/password"]);
}

std::string ServicesID::getDesc() {
	return "Allows identification/authentification to services.";
}

extern "C" Module* spawn() {
	return new ServicesID;
}