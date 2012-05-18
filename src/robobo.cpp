#include "robobo.h"

int main(int argc, char** argv) {
	std::string workingDir = argv[0];
	workingDir = workingDir.substr(0, workingDir.find_last_of('/'));
	std::string confDir = workingDir;
	std::string confName = "robobo.conf";
	unsigned short debugLevel = 0;
	bool logDump = false;
	if (argc > 1) {
		unsigned int i = 0;
		while (++i < argc) {
			std::string currArg = argv[i];
			if (currArg == "--help" || currArg == "-h" || currArg == "-?") {
				std::cout << "RoBoBo IRC Bot Help" << std::endl;
				std::cout << std::endl;
				std::cout << "Start the bot without any parameters to start it normally." << std::endl;
				std::cout << "--help: Displays this help notice" << std::endl;
				std::cout << "--version: Displays bot version" << std::endl;
				std::cout << "--debug: Starts the bot in debug mode (foreground, sends information to standard output)" << std::endl;
				std::cout << "\tSpecify level 1, 2, or 3 after it for more information" << std::endl;
				std::cout << "--confname: Specifies the name of the configuration file (default: robobo.conf)" << std::endl;
				std::cout << "--confdir: Specifies the directory of the configuration file (default: same as executable)" << std::endl;
				return 0;
			}
			if (currArg == "--version" || currArg == "-v") {
				std::cout << "RoBoBo IRC Bot, version 3.0.0 Development" << std::endl;
				return 0;
			}
			if (currArg == "--debug" || currArg == "-d") {
				i++;
				if (i >= argc)
					debugLevel = 1;
				else {
					std::istringstream specifiedDebug (argv[i]);
					specifiedDebug >> debugLevel;
				}
				continue;
			}
			if (currArg == "--log" || currArg == "-l") {
				logDump = true;
				continue;
			}
			if (currArg == "--confname") {
				i++;
				if (i >= argc)
					std::cerr << "An error occurred parsing command line arguments: --confname was not given a parameter, using default" << std::endl;
				else
					confName = argv[i];
				continue;
			}
			if (currArg == "--confdir") {
				i++;
				if (i >= argc)
					std::cerr << "An error occurred parsing command line arguments: --confdir was not given a parameter, using default" << std::endl;
				else
					confDir = argv[i];
				continue;
			}
			// In the case that we receive an unrecognized command line input, return an error but continue
			std::cerr << "An error occurred parsing command line arguments: unknown argument " << argv[i] << std::endl;
		}
	}
	// We're done parsing command line arguments; it's time to start!
	Base botInstance (workingDir, confDir, confName, debugLevel, logDump);
	botInstance.readConfiguration();
	botInstance.startQueueThread();
	botInstance.loadModules();
	botInstance.connectServers();
	botInstance.checkModules();
	// If checkModules returns, the bot is shutting down, so kill all the things
	botInstance.unloadEverything();
}