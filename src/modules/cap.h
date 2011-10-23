class CapModule : public Module {
	public:
		CapModule(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug);
		virtual ~CapModule();
		virtual void registerCap(std::string moduleName, std::string capCommand);
		virtual void blockCap(std::string server, std::string moduleName);
		virtual void continueCap(std::string server, std::string moduleName);
};

class CapClient : public Module {
	public:
		CapClient(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug);
		virtual ~CapClient();
		virtual void onCapAccept(std::string server, std::string capCommand);
};

CapModule::CapModule(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug) : Module(modConf, modFace, modName, dir, debug) {}

CapModule::~CapModule() {}

void CapModule::registerCap(std::string moduleName, std::string capCommand) {}

void CapModule::blockCap(std::string server, std::string moduleName) {}

void CapModule::continueCap(std::string server, std::string moduleName) {}

CapClient::CapClient(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug) : Module(modConf, modFace, modName, dir, debug) {}

CapClient::~CapClient() {}

void CapClient::onCapAccept(std::string server, std::string capCommand) {}