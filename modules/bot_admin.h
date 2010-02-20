class AdminHook : public Module {
	public:
		virtual ~AdminHook();
		virtual std::vector<std::vector<std::string> > adminCommands();
		virtual void onAdminCommand(std::string server, std::string nick, std::string command, std::string message, bool dcc, bool master);
};

class AdminMod {
	public:
		virtual ~AdminMod();
		virtual void sendVerbose(int verboseLevel, std::string message);
};

AdminHook::~AdminHook() {}

std::vector<std::vector<std::string> > AdminHook::adminCommands() { return std::vector<std::vector<std::string> > (); }

void AdminHook::onAdminCommand(std::string server, std::string nick, std::string command, std::string message, bool dcc, bool master) {}

AdminMod::~AdminMod() {}

void AdminMod::sendVerbose(int verboseLevel, std::string message) {}