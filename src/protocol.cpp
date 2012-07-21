#include "protocol.h"

Protocol::Protocol(std::string server, std::map<std::string, std::string> conf, std::string workDir, bool dumpLogs, unsigned short debug, Base* botptr) : serverName(server), workingDir(workDir), config(conf), log(dumpLogs), debugLevel(debug), bot(botptr) {}

Protocol::~Protocol() {}