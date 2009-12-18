#include "main.h"

#include <dlfcn.h>

class ModuleInterface; // forward-declare so it can be used in modules and server
#include "connection.h" // declare other classes that use it
#include "modules.cpp"
#include "modinterface.cpp" // and now include it
#include "server.cpp"
#include "config.cpp"