#include "main.h"

#include <dlfcn.h>

class ModuleInterface; // forward-declare so it can be used in modules and server
#include "modules.cpp"
#include "server.cpp"
#include "modinterface.cpp" // and now include it
#include "config.cpp"