#pragma once
#include "main.h"

#include "socket.h"

class Base; // This needs to be forward-declared for the Base* pointer in Module; the Base class is included below.
enum LoadResult { LOAD_SUCCESS, LOAD_ALREADYLOADED, LOAD_OPEN_ERROR, LOAD_INCOMPATIBLE, LOAD_NODEPENDS, LOAD_FAILURE };

/** Module priority
 * A flag used by modules to help determine the order in which module hooks are called.
 */
enum Priority { PRI_HIGH, PRI_MEDIUM_HIGH, PRI_NORMAL, PRI_MEDIUM_LOW, PRI_LOW };

/// Define the spawn function for module files
#define MODULE_SPAWN(modName) extern "C" Module* spawn(std::string moduleName, std::map<std::string, std::string> config, std::string workingDir, unsigned short debugLevel, Base* botptr) {\
		return new modName (moduleName, config, workingDir, debugLevel, botptr);\
	}

/** Message action
 * A flag returned by modules with received messages such that messages can be ignored or not.
 */
typedef bool MsgAction;
const bool MSG_CONTINUE = true;
const bool MSG_IGNORE = false;

class Module {
	public:
		/** Module constructor
		 * Fills the Module class with the basic things it needs to function
		 * @param modName The name of the module
		 * @param conf The module configuration
		 * @param workDir The working directory of the bot relative to the directory from which it was started
		 * @param debug The current debug flag of the bot
		 * @param botptr A pointer to the bot base for communication with the rest of the bot
		 */
		Module(std::string modName, std::map<std::string, std::string> conf, std::string workDir, unsigned short debug, Base* botptr);
		
		/** Module destructor
		 * As this is the base module class from which modules will be derived, the destructor is virtual.
		 */
		virtual ~Module();
		
		/// The module's priority
		virtual Priority priority();
		
		/** Module API version
		 * This function checks whether the module API version for which the module is written is compatible with
		 * the existing API version.
		 * @return API version
		 */
		virtual unsigned int apiVersion() = 0;
		
		/** Load completion hook
		 * This function is called when the module has finished being loaded so that modules can do whatever startup
		 * things they need to.
		 * @return true if the module loaded successfully; false if it did not.
		 */
		virtual bool onLoadComplete();
		
		/** Unload hook
		 * This function is called when the module is about to be unloaded.
		 */
		virtual void onUnload();
		
		/** Rehash hook
		 * This function is called when the bot has just rehashed.  This allows the bot to react to changes in the
		 * configuration if it has a special setup to do so.
		 */
		virtual void onRehash();
		
		/** Module load hook
		 * This function is called when a different module loads, allowing modules to react to the loading of another
		 * module.
		 * @param modName The name of the module being loaded.
		 */
		virtual void onModuleLoad(std::string modName);
		
		/** Module unload hook
		 * This function is called when a different module unloads, allowing modules to react to the unloading of
		 * another module, e.g. to kill itself if it depends on the module being unloaded.
		 * @param modName The name of the module being unloaded.
		 */
		virtual void onModuleUnload(std::string modName);
		
		/** Keep bot alive
		 * If, for some reason, the module needs to keep the bot alive even after all servers have disconnected, it
		 * can do so by returning true here.  This should be a rare case, but it could have some use.
		 * @return Whether to force the bot staying alive
		 */
		virtual bool forceKeepAlive();
		
		/** Rehash function
		 * This function replaces the existing configuration of the module with a new one loaded from the configuration.
		 * The onRehash hook is called immediately after.
		 * @param conf The new configuration read from the config file.
		 * @see onRehash
		 */
		void rehash(std::map<std::string, std::string> conf);
		
		/** End debug notifier
		 * If, at any time, the bot leaves debug mode, this function will be called in all modules to set the debug
		 * level to 0.
		 * @see debugLevel
		 */
		void endDebug();
		
		
		
		
		
		/** Description
		 * Allows modules to provide a human-readable description of their function.
		 * @return A human-readable description of the module
		 */
		virtual std::string description();
		
		/** Provided services
		 * Allows the bot core to determine which services this module provides.
		 * @return A list of services provided by the module
		 */
		virtual std::list<std::string> provides();
		
		/** Required services
		 * Allows the bot core to determine which services must be provided by other modules for this module to function.
		 * @return A list of services required by the module
		 */
		virtual std::list<std::string> requires();
		
		/** Supported services
		 * Allows the bot core to determine which services this module makes use of but does not require.
		 * @return A list of services used by the module
		 */
		virtual std::list<std::string> supports();
	protected:
		/** Utility variables
		 * moduleName: The name the bot core uses to reference this module
		 * workingDir: The directory of the bot relative to where it was started from
		 */
		const std::string moduleName, workingDir;
		
		/// Holds the module configuration as provided in the configuration file
		std::map<std::string, std::string> config;
		
		/// The current debug level of the bot, used to determine how much output to give
		unsigned short debugLevel;
		
		
		
	private:
		/// Holds a pointer to the bot base for communication with protocol modules and other normal modules
		Base* bot;
};

#include "base.h"