#include "robobo.h"
#include <string.h> // C strings to handle args

int main(int argc, char** argv) {
	std::string confDir = ".";
	std::string confName = "robobo.conf";
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
				std::cout << "\t--version: display RoBoBo's version and exit" << std::endl;
				exitAfter = true;
			} else if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
				std::cout << "RoBoBo-IRC-BoBo Pre-alpha Development Version" << std::endl;
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
			}
			std::cout << std::endl; // add a newline after a parameter's output
		}
		if (exitAfter)
			return 0;
	}
	new ModuleInterface (ConfigReader (confName, confDir)); //run actual bot
	pthread_exit(NULL);
}