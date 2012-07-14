#pragma once
#include "main.h"

#include "socket.h"

class Base; // This needs to be forward-declared for the Base* pointer in Module; the Base class is included below.
enum LoadResult { LOAD_SUCCESS, LOAD_ALREADYLOADED, LOAD_OPEN_ERROR, LOAD_INCOMPATIBLE, LOAD_NODEPENDS, LOAD_FAILURE };
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
		virtual ~Module();
		virtual Priority priority();
		virtual unsigned int apiVersion() = 0;
		virtual bool onLoadComplete();
		virtual void onUnload();
		virtual void onRehash();
		virtual void onModuleLoad(std::string modName);
		virtual void onModuleUnload(std::string modName);
		virtual bool forceKeepAlive();
		void rehash(std::map<std::string, std::string> conf);
		void endDebug();
		
		
		
		virtual std::string description();
		virtual std::list<std::string> provides();
		virtual std::list<std::string> requires();
		virtual std::list<std::string> supports();
	protected:
		const std::string moduleName, workingDir;
		std::map<std::string, std::string> config;
		unsigned short debugLevel;
		
		
	private:
		Base* bot;
};

#include "base.h"