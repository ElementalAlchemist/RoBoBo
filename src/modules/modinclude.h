#ifndef MODINCLUDE_H
#define MODINCLUDE_H
#include "../main.h"
#include "../modules.h"

#define MODULE_SPAWN(name) extern "C" Module* spawn(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug) {\
	return new name (modConf, modFace, modName, dir, debug); \
}
#endif
