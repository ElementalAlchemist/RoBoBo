#include "robobo.h"

int main(int argc, char** argv) {
	std::string confDir = argv[0]; // look in directory with robobo executable by default
	confDir = confDir.substr(0, confDir.find_last_of('/'));
	std::string confName = "robobo.conf"; // look for robobo.conf by default
	unsigned short debug = 0;
	if (argc > 1) { // analyze arguments
		bool exitAfter = false;
		for (int i = 1; i < argc; i++) { // iterate through all arguments
			if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-?") == 0) {
				std::cout << "RoBoBo-IRC-BoBo IRC Bot Help" << std::endl;
				std::cout << std::endl;
				std::cout << "RoBoBo can connect to IRC servers.  All of its functionality outside of" << std::endl << "connecting to servers is provided by loaded modules." << std::endl;
				std::cout << "See the README file for more information." << std::endl;
				std::cout << std::endl;
				std::cout << "Some command line arguments are provided to perform certain actions.  With no" << std::endl << "parameters, the bot will run as a bot.  With some command line arguments, the" << std::endl << "functionality of the bot can be changed." << std::endl;
				std::cout << "Command Line Arguments:" << std::endl;
				std::cout << "\t--help: display this help and exit" << std::endl;
				std::cout << "\t\t-h: same as --help" << std::endl;
				std::cout << "\t\t-?: same as --help" << std::endl;
				std::cout << "\t--version: display RoBoBo's version and exit" << std::endl;
				std::cout << "\t\t-v: same as --version" << std::endl;
				std::cout << "\t--confdir <directory>: make RoBoBo look in the specified directory for the configuration instead of current directory" << std::endl;
				std::cout << "\t--confname <filename>: make RoBoBo look for the specified file in the conf directory for configuration information" << std::endl;
				std::cout << "\t--debug: make RoBoBo enter debug mode" << std::endl;
				std::cout << "\t\t-d: same as --debug" << std::endl;
				exitAfter = true;
			} else if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
				std::cout << "RoBoBo-IRC-BoBo Version 2.0.0 Beta 1" << std::endl;
				exitAfter = true;
			} else if (strcmp(argv[i], "--confdir") == 0) {
				if (++i >= argc) {
					std::cout << "An argument was not specified for the --confdir argument." << std::endl;
					return 0;
				}
				confDir = argv[i];
				std::cout << "Looking for the configuration file in " << confDir << std::endl;
			} else if (strcmp(argv[i], "--confname") == 0) {
				if (++i >= argc) {
					std::cout << "An argument was not specified for the --confname argument." << std::endl;
					return 0;
				}
				confName = argv[i];
				std::cout << "Looking for a configuration file named " << confName << std::endl;
			} else if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-d") == 0) {
				if (i+1 >= argc || !(strcmp(argv[i+1], "0") == 0 || strcmp(argv[i+1], "1") == 0 || strcmp(argv[i+1], "2") == 0 || strcmp(argv[i+1], "3") == 0))
					debug = 1;
				else {
					std::istringstream debugNum (argv[++i]);
					debugNum >> debug;
				}
				std::cout << "Setting debug mode with level " << debug << std::endl;
			}
			std::cout << std::endl; // add a newline after a parameter's output
		}
		if (exitAfter)
			return 0;
	}
	std::cout << "RoBoBo-IRC-BoBo started." << std::endl;
	if (debug == 0)
		daemon(1,0);
	new Base (confDir, confName, debug); //run actual bot
	pthread_exit(NULL);
}