#include "main.h"

class ConfigReader;
class ModuleInterface; // forward-declare so it can be used in modules and server
#include "connection.h" // declare other classes that use it
#include "modules.cpp"
#include "config.cpp"
#include "modinterface.cpp" // and now include it
#include "server.cpp"

#include <string.h> // C strings to handle args