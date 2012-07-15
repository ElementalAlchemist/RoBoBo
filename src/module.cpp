#include "module.h"

Module::Module(std::string modName, std::map<std::string, std::string> conf, std::string workDir, unsigned short debug, Base* botptr) : moduleName(modName), workingDir(workDir), config(conf), debugLevel(debug), bot(botptr) {}

Module::~Module() {}
// This is just here because we need a virtual destructor.