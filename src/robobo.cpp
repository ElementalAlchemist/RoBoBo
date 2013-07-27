#include "robobo.h"

int main(int argc, char** argv) {
	std::string workingDir (argv[0]);
	workingDir = workingDir.substr(0, workingDir.find_last_of('/'));
	std::string confName ("robobo.conf");
	unsigned short debugLevel = 0;
	if (argc > 1) {
		opterr = 0;
		option validOptions[] = {
			{"help", 0, nullptr, 'h'},
			{"version", 0, nullptr, 'v'},
			{"debug", 2, nullptr, 'd'},
			{"confname", 1, nullptr, 1}
		};
		int optCode;
		while ((optCode = getopt_long(argc, argv, "h?vd::", validOptions, nullptr)) != EOF) {
			switch (optCode) {
				case 'h':
				case '?':
					std::cout << "RoBoBo IRC Bot Help" << std::endl;
					std::cout << std::endl;
					std::cout << "Start the bot without any parameters to start it normally." << std::endl;
					std::cout << "--help/-h: Displays this help notice" << std::endl;
					std::cout << "--version/-v: Displays bot version" << std::endl;
					std::cout << "--debug/-d: Starts the bot in debug mode (foreground, sends information to standard output)" << std::endl;
					std::cout << "\tSpecify level 1, 2, or 3 after it for more information" << std::endl;
					std::cout << "\tIf the debug argument is given with no level, level 1 is assumed." << std::endl;
					std::cout << "--confname: Specifies the name of the configuration file, along with a directory" << std::endl;
					std::cout << "\tif it's not in the main robobo directory (default: robobo.conf)" << std::endl;
					std::cout << "\te.g. '--confname ../test.conf' or '--confname /etc/robobo.conf'" << std::endl;
					std::cout << "\tRelative paths are relative to the main robobo directory." << std::endl;
					return 0;
				case 'v':
					std::cout << "RoBoBo IRC Bot " << BotVersion << std::endl;
					return 0;
				case 'd':
					if (optarg == 0)
						debugLevel = 1;
					else {
						std::istringstream debugStr (optarg);
						debugStr >> debugLevel;
					}
					if (debugLevel > 4)
						debugLevel = 4;
					break;
				case 1:
					confName = optarg;
			}
		}
	}
	// Default the configuration to the working directory if an absolute path is not provided
	// If a relative path is provided, it should be relative to the working directory
	Config* configuration = Config::getHandle();
	LogManager* logger = LogManager::getHandle();
	logger->setLogDir(workingDir + "/logs/");
	logger->setDefaultLevel(static_cast<LogLevel> (debugLevel));
	configuration->setMainConfigFile(confName);
	configuration->setWorkingDirectory(workingDir);
	configuration->addRehashNotify(std::bind(&LogManager::updateLogFiles, logger));
	configuration->readConfig();
	// TODO: module manager and server manager
	
	// Add signal handlers
	sigset_t signalSet;
	sigemptyset(&signalSet);
	sigaddset(&signalSet, SIGHUP);
	sigaddset(&signalSet, SIGUSR1);
	struct sigaction sigHandle;
	sigHandle.sa_handler = [](int signum) {
		if (signum == SIGHUP) {
			daemon(1, 0);
			LogManager* logger = LogManager::getHandle();
			logger->setDefaultLevel(LOG_NONE);
		} else if (signum == SIGUSR1) {
			Config* config = Config::getHandle();
			config->readConfig();
		}
	};
	sigHandle.sa_mask = signalSet;
	sigHandle.sa_flags = 0;
	const struct sigaction* sigPtr = &sigHandle;
	sigaction(SIGHUP, sigPtr, nullptr);
	sigaction(SIGUSR1, sigPtr, nullptr);
	
	/*
	bot = new Base (workingDir, confDir, confName, debugLevel, logDump);
	bot->readConfiguration();
	bot->loadModules();
	bot->connectServers();
	if (debugLevel == 0)
		daemon(1, 0);
	bot->checkServers();
	// If checkModules returns, the bot is shutting down, so kill all the things
	bot->unloadEverything();
	delete bot;
	*/
}