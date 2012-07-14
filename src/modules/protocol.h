#include "../main.h"
#include "../protocol.h"

#define PROTOCOL_SPAWN(modName) extern "C" Protocol* spawn(std::string server, std::map<std::string, std::string> conf, std::string workDir, bool dumpLogs, unsigned short debug, Base* botptr) {\
	return new modName (server, conf, workDir, dumpLogs, debug, botptr);\
}