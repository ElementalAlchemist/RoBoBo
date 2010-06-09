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
/* adminCommands(std::vector<std::vector<std::string> >)
This function allows modules to specify which admin commands they provide.  This allows the admin module to call the
admin command function (onAdminCommand) when one of the commands is given.  The function should return a vector of vectors
of strings, as such:
vector -> {
	vector -> {
		"command1" // the command itself, in all lowercase
		"This is a sample command that actually does absolutely nothing." // a short description of the command
		"Syntax: command1" // Elements 2 and above in the vector are the help for the command
		"This command secretly does nothing." // Each element is sent as one line of the help
		"But don't tell anyone! ;)"
	}
	vector -> {
		"anothercommand" // this bot supports two commands
		"This command calls the first command."
		"Syntax: anothercommand"
		"Blah."
	}
}
Using push_back, the strings should be added to the inner vector in that order.  Vectors added to the outer vector can
be added in any order.
*/

void AdminHook::onAdminCommand(std::string server, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master) {}
/* onAdminCommand(std::string, std::string, std::string, std::string, dccSender*, bool)
This function is called by the admin module whenever a command that you told the admin module in adminCommands().
The parameters are provided as follows:
- server: the server (as known by the bot) to which the user sending the command is connected
- nick: the nick the user sending the command is using
- command: the command given (you won't need to check this if you only gave one command in adminCommands())
- message: All of the parameters given to the command (i.e. the entire message starting immediately after the space after the command)
- dccMod: A pointer to the DCC chat module.  If the command is given via DCC, the dccMod is a pointer to the DCC mod.  Using the default DCC
	module provided by the bot distribution, the ID you would send to to reply to the command is server/nick.
	If the command is not given to the admin via DCC, the dccMod pointer will be NULL.
- master: Whether a bot master sent the command.  If true, the bot master sent the command.  If false, an admin who is not the bot master sent it.
*/

AdminMod::~AdminMod() {}

void AdminMod::sendVerbose(int verboseLevel, std::string message) {}
/* sendVerbose(int, std::string)
This function sends the message specified to all admins with a verbose level of at least verboseLevel.
*/