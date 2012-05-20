#include "main.h"

enum Priority { PRI_HIGH, PRI_MEDIUM_HIGH, PRI_NORMAL, PRI_MEDIUM_LOW, PRI_LOW };

#define MODULE_SPAWN(modName) extern "C" Module* spawn(std::string moduleName, std::map<std::string, std::string> config, std::string workingDir, unsigned short debugLevel, Base* botptr) {\
		return new modName (moduleName, config, workingDir, debugLevel, botptr);\
	}

typedef bool MsgAction;
const bool MSG_CONTINUE = true;
const bool MSG_IGNORE = false;

class Module {
	public:
		Module(std::string modName, std::map<std::string, std::string> conf, std::string workDir, unsigned short debug, Base* botptr);
		const Priority priority;
		unsigned int apiVersion() = 0;
		bool onLoadComplete();
		void onRehash();
		void onModuleLoad(std::string modName);
		void onModuleUnload(std::string modName);
		
		// TODO: module hooks
		
		std::string description();
		std::list<std::string> provides();
		std::list<std::string> requires();
		std::list<std::string> supports();
	protected:
		std::string moduleName;
		std::map<std::string, std::string> config;
		std::string workingDir;
		unsigned short debugLevel;
	private:
		Base* bot;
};