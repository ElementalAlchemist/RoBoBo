#include "robobo.h"

Base* bot;

void sigHandler(int signum) {
	// Signal calls 
	if (signum == SIGHUP) {
		daemon(1, 0);
		bot->endDebug();
	} else if (signum == SIGUSR1)
		bot->sigRehash();
}

int main(int argc, char** argv) {
	std::string workingDir = argv[0];
	workingDir = workingDir.substr(0, workingDir.find_last_of('/'));
	std::string confDir = workingDir;
	std::string confName = "robobo.conf";
	unsigned short debugLevel = 0;
	bool logDump = false;
	if (argc > 1) {
		opterr = 0;
		option validOptions[] = {
			{"help", 0, NULL, 'h'},
			{"version", 0, NULL, 'v'},
			{"debug", 2, NULL, 'd'},
			{"log", 0, NULL, 'l'},
			{"confname", 1, NULL, 1},
			{"confdir", 1, NULL, 2}
		};
		int optCode;
		while ((optCode = getopt_long(argc, argv, "h?vd::l", validOptions, NULL)) != EOF) {
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
					std::cout << "--confname: Specifies the name of the configuration file (default: robobo.conf)" << std::endl;
					std::cout << "--confdir: Specifies the directory of the configuration file (default: same as executable)" << std::endl;
					return 0;
				case 'v':
					std::cout << "RoBoBo IRC Bot version 3.0.0 Development" << std::endl;
					return 0;
				case 'd':
					if (optarg == 0)
						debugLevel = 1;
					else {
						std::istringstream debugStr (optarg);
						debugStr >> debugLevel;
					}
					break;
				case 'l':
					logDump = true;
					break;
				case 1:
					confName = optarg;
					break;
				case 2:
					confDir = optarg;
			}
		}
	}
	// Add signal handlers
	sigset_t signalSet;
	sigemptyset(&signalSet);
	sigaddset(&signalSet, SIGHUP);
	sigaddset(&signalSet, SIGUSR1);
	struct sigaction sigHandle;
	sigHandle.sa_handler = sigHandler;
	sigHandle.sa_mask = signalSet;
	sigHandle.sa_flags = 0;
	const struct sigaction* sigPtr = &sigHandle;
	sigaction(SIGHUP, sigPtr, NULL);
	sigaction(SIGUSR1, sigPtr, NULL);
	// We're done parsing command line arguments; it's time to start!
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
}