#pragma once
#include "main.h"
#include <dlfcn.h>

/** Load result flags
 * A set of flags, one of which is returned by loadModule to show either that the module was loaded successfully
 * or why the module could not be loaded.
 */

#include "socket.h"
#include "protocol.h"
#include "module.h"

// The spawn function needs a typedef so that it can be called after a dlsym.
// This typedef is used to cast the result of dlsym so that it can be used. (see Base::loadModule)
typedef void* (module_spawn_t)(std::string, std::map<std::string, std::string>, std::string, unsigned short, Base*);
typedef void* (protocol_spawn_t)(std::string, std::map<std::string, std::string>, std::string, bool, unsigned short, Base*);
typedef void* (socket_spawn_t)();

class Base {
	public:
		/** Constructor for the Base class
		 * This function serves as the constructor of Base, accepting parameters for various things
		 * it needs to be able to start the bot.
		 * @param working The working directory of the bot (i.e. the relative path to the executable
		 * from where the bot was started)
		 * @param config The relative directory of the config file from the working directory
		 * @param configFileName The name of the configuration file
		 * @param debug The debug level at which the bot was started
		 * @param log Whether the bot is to generate logs; sent to protocol modules
		 */
		Base(std::string working, std::string config, std::string configFileName, unsigned short debug, bool log);
		
		/** Configuration file reader
		 * Executed as part of the bot's startup routine, this opens the config file and reads it,
		 * storing it in various variables throughout the Base class to be read as needed.
		 */
		void readConfiguration();
		
		/** Startup module loader
		 * Executed as part of the bot's startup routine, this loads all of the modules specified in
		 * the configuration to be loaded on the bot's startup.
		 */
		void loadModules();
		
		/** Startup module load completion
		 * Executed from loadModules, this function is called for each module after all of the modules
		 * have been loaded on startup and returns true if everything went well.
		 * @param modName The name of the module being checked
		 * @param modptr A pointer to the module class for the module in question
		 */
		bool completeStartupLoad(std::string modName, Module* modptr);
		
		/** Server connecter
		 * Executed as part of the bot's startup routine, this loads all of the needed protocol
		 * modules and connects to the servers specified in the configuration to be connected on
		 * startup.
		 */
		void connectServers();
		
		/** Server check thread
		 * This function serves as a thread running continuously once the bot is started up, and
		 * it continues to run until it determines that the bot should shut down.  Its purpose is
		 * to detect servers that have been disconnected and reconnect them if appropriate.
		 */
		void checkServers();
		
		/** Destroyer of everything
		 * This function runs after checkServers exits, and its purpose is to safely unload everything.
		 */
		void unloadEverything();
		
		/** Module loader
		 * This function loads modules and verifies that all is well with them.  As part of the loading
		 * process, a module may signal to this function that it cannot be loaded.
		 * @param modName The name of the module to load
		 * @return A code indicating if the load was successful or the cause of error
		 */
		LoadResult loadModule(std::string modName);
		
		/** Module unloader
		 * This function unloads modules.
		 * @param modName The name of the module to unload
		 * @param wasLoaded A parameter used by the startup loadModules function to tell this function not
		 * to call the module unload hooks
		 */
		void unloadModule(std::string modName, bool wasLoaded);
		
		/** Server connecter
		 * This function loads a protocol module if necessary and connects to the specified server.
		 * @param server The name of the server to connect
		 */
		void connectServer(std::string server);
		
		/** Server disconnecter
		 * This function disconnects from the specified server and, if necessary, unloads the appropriate
		 * protocol module.
		 * @param server The name of the server to disconnect
		 * @param reason The reason to disconnect
		 */
		void disconnectServer(std::string server, std::string reason = "");
		
		/** Socket module loader
		 * This function loads a socket module requested by a protocol module and returns the socket.
		 * @param sockettype The name of the type of socket to load
		 */
		Socket* loadSocket(std::string sockettype);
		
		/** Socket module unloader
		 * This function unloads a socket module when the protocol module is done with it (e.g. if the server
		 * is disconnecting or some such)
		 * @param sockettype The name of the type of socket being unloaded
		 * @param socketptr A pointer to the socket class needing removed
		 */
		void unloadSocket(std::string sockettype, Socket* socketptr);
		
		/** Rehash
		 * This function reloads the configuration and distributes the information as necessary.
		 */
		void rehash();
		
		/** End debug
		 * This function is called when the signal is received and the bot is daemonized in debug mode, so that it can stop being
		 * in debug mode.
		 */
		void endDebug();
		
		
		
		/** Module list
		 * Lists loaded modules.
		 * @return a list of module names
		 */
		std::list<std::string> moduleList();
		
		/** Provided services
		 * Lists services provided by modules.
		 * @return a list of services
		 */
		std::list<std::string> providedServices();
		
		/** Provided service check
		 * Checks whether a given service is provided by a module.
		 * @param service The service for which to check
		 * @return true if the service is provided by a module, false otherwise
		 */
		bool serviceIsProvided(std::string service);
		
		/** Service provider list
		 * Lists modules that provide a given service.
		 * @param service The service for which we want providers
		 * @return a list of module names
		 */
		std::list<std::string> serviceProviders(std::string service);
		
		/** Service user list
		 * Lists services that use a given service.
		 * @param service The service for which we want users
		 * @return a list of module names
		 */
		std::list<std::string> serviceUsers(std::string service);
		
		/** Refresh services
		 * Instructs the bot core that the module's requirements have changed and that the core should reload the list of
		 * services it provides/requires/supports.
		 * @param module The module requesting the rescan
		 */
		void refreshServices(std::string module);
	private:
		/** Directory variables
		 * workingDir: The directory of the executable relative to the directory from which robobo was started
		 * configDir: The directory from workingDir to the config
		 * configName: The name of the config file
		 */
		const std::string workingDir, configDir, configName;
		/// The debug level at which the bot is running
		const unsigned short debugLevel;
		/// Whether to dump logs (specified by the --log startup option)
		const bool logDump;
		/// Whether the bot is in startup mode; true while starting up, set to false once everything is running
		bool startup;
		/** Config variables
		 * serverConfig: holds server configuration
		 * moduleConfig: holds module configuration
		 */
		std::map<std::string, std::map<std::string, std::string>> serverConfig, moduleConfig;
		/** Startup variables
		 * startupServers: holds a list of servers to connect on startup
		 * startupModules: holds a list of modules to load on startup
		 */
		std::list<std::string> startupServers, startupModules;
		/// Servers currently loaded
		std::map<std::string, Protocol*> servers;
		/// The modules currently loaded, separated by priority
		std::map<std::string, Module*> highModules, mediumHighModules, normalModules, mediumLowModules, lowModules;
		/// The module file handles returned by dlopen() so that modules can be properly unloaded
		std::unordered_map<std::string, void*> moduleFiles;
		/// Stored module priority by module name
		std::map<std::string, Priority> modulePriority;
		/** Service variables
		 * moduleServices: holds a list of modules providing each given service
		 * moduleRequires: holds a list of modules requiring each given service
		 * moduleSupports: holds a list of modules using each given service
		 */
		std::map<std::string, std::list<std::string>> moduleServices, moduleRequires, moduleSupports;
		/// Holds a description for each module
		std::map<std::string, std::string> moduleDescriptions;
		/// The protocol file handles returned by dlopen() for each protocol type
		std::unordered_map<std::string, void*> protocolFiles;
		/// Which protocol modules are of which type
		std::unordered_map<std::string, std::set<std::string>> protocolTypes;
		/// The socket file handles returned by dlopen() for each socket type
		std::unordered_map<std::string, void*> socketFiles;
		/// The number of a given type of socket open
		std::unordered_map<std::string, size_t> socketCounts;
		/// The mutex ensuring that module hooks are called one at a time
		std::mutex modHookMutex;
};