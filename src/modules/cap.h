class CapModule : public Module {
	public:
		CapModule(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug);
		void registerCap(std::string moduleName, std::string capCommand);
		void blockCap(std::string server, std::string moduleName);
		void continueCap(std::string server, std::string moduleName);
};

class CapClient : public Module {
	public:
		CapClient(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug);
		void onCapAccept(std::string server, std::string capCommand);
};

CapModule::CapModule(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug) : Module(modConf, modFace, modName, dir, debug) {}

void CapModule::registerCap(std::string moduleName, std::string capCommand) {}

void CapModule::blockCap(std::string server, std::string moduleName) {}

void CapModule::continueCap(std::string server, std::string moduleName) {}

CapClient::CapClient(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug) : Module(modConf, modFace, modName, dir, debug) {}

void CapClient::onCapAccept(std::string server, std::string capCommand) {}