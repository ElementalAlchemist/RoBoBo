#include "module.h"

Module::Module(std::string modName, std::map<std::string, std::string> conf, std::string workDir, unsigned short debug, Base* botptr) : moduleName(modName), workingDir(workDir), config(conf), debugLevel(debug), bot(botptr) {}
// The default priority is PRI_NORMAL but can be overridden in the subclass's constructor

Module::~Module() {}
// This is just here because we need a virtual destructor.