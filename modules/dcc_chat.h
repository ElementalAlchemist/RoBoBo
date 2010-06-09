class dccChat : public Module {
	public:
		virtual ~dccChat();
		virtual void onDCCReceive(std::string dccid, std::string message);
		virtual void onDCCEnd(std::string dccid);
};

class dccSender : public Module {
	public:
		virtual ~dccSender();
		virtual void dccSend(std::string dccid, std::string message);
		virtual bool hookDCCMessage(std::string modName, std::string hookMsg);
		virtual void unhookDCCSession(std::string modName, std::string dccid);
		virtual std::vector<std::string> getConnections();
		virtual void closeDCCConnection(std::string dccid);
};

dccChat::~dccChat() {}

void dccChat::onDCCReceive(std::string dccid, std::string message) {}
/* onDCCReceive(std::string, std::string)
This function is called whenever the DCC chat module receives a DCC message that should be sent to your module
(the message starting with hookMsg and all messages after until unhookDCCSession is called).  It gives the DCC
ID of the sender (using the default DCC chat module, server/nick) and the message sent to the bot.
*/

void dccChat::onDCCEnd(std::string dccid) {}
/* onDCCEnd(std::string)
This function is called when a DCC chat session is terminated.  The DCC ID of the user whose chat was terminated
is provided.
*/

dccSender::~dccSender() {}

void dccSender::dccSend(std::string dccid, std::string message) {}
/* dccSend(std::string, std::string)
This function tells the DCC chat module to send the provided message to the DCC session associated with the provided
DCC ID.
*/

bool dccSender::hookDCCMessage(std::string modName, std::string hookMsg) { return false; }
/* hookDCCMessage(std::string, std::string)
This function tells the DCC chat module to hook any message starting with hookMsg to the module modName.  To hook messages
to the module calling the function, modName should be sent as the Module variable moduleName, which is provided in every
module as the internal bot name for your module.
This function returns true if the message could be hooked, and false if it couldn't.
*/

void dccSender::unhookDCCSession(std::string modName, std::string dccid) {}
/* unhookDCCSession(std::string, std::string)
This function tells the DCC chat module that it shouldn't send any more messages from the DCC chat session associated with
the provided DCC ID to the module modName.  To unhook the DCC session from the module calling the function, the module should
use the Module variable moduleName, which is provided in every module as the internal bot name for your module.
*/

std::vector<std::string> dccSender::getConnections() {
	return std::vector<std::string> ();
}
/* getConnections()
This function returns a vector of the DCC IDs of all connected DCC chat sessions.
*/

void dccSender::closeDCCConnection(std::string dccid) {}
#include <unistd.h>
/* closeDCCConnection(std::string)
This function causes the bot to close the DCC connection associated with the given DCC ID.
*/