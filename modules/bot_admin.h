#include "dcc_chat.h"

class AdminHook : public dccChat {
	public:
		virtual ~AdminHook();
		virtual std::vector<std::vector<std::string> > adminCommands();
		virtual void onAdminCommand(std::string server, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master);
};

class AdminMod : public dccChat {
	public:
		virtual ~AdminMod();
		virtual void sendVerbose(int verboseLevel, std::string message);
};

AdminHook::~AdminHook() {}

std::vector<std::vector<std::string> > AdminHook::adminCommands() { return std::vector<std::vector<std::string> > (); }

void AdminHook::onAdminCommand(std::string server, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master) {}

AdminMod::~AdminMod() {}

void AdminMod::sendVerbose(int verboseLevel, std::string message) {}