class CapModule : public Module {
	public:
		void registerCap(std::string moduleName, std::string capCommand);
		void blockCap(std::string server, std::string moduleName);
		void continueCap(std::string server, std::string moduleName);
};

class CapClient : public Module {
	public:
		void onCapExists(std::string server, std::string capCommand);
		void onCapAccept(std::string server, std::string capCommand);
};

void CapModule::registerCap(std::string moduleName, std::string capCommand) {}

void CapModule::blockCap(std::string server, std::string moduleName) {}

void CapModule::continueCap(std::string server, std::string moduleName) {}

void CapClient::onCapExists(std::string server, std::string capCommand) {}

void CapClient::onCapAccept(std::string server, std::string capCommand) {}