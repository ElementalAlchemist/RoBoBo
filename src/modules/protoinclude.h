#include "../main.h"
#include "../protocol.h"

#define PROTOCOL_SPAWN(name) extern "C" Protocol* spawn(std::string serverAddr, std::tr1::unordered_map<std::string, std::string> config, Base* base, unsigned short debugLevel) {\
	return new name (serverAddr, config, base, debugLevel);\
}