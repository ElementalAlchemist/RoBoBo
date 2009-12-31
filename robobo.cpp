#include "robobo.h"

int main(int argc, char** argv) {
	if (argc > 1) { // analyze arguments
		if (strcmp(argv[1], "--help") == 0) {
			// put help here eventually
			return 0;
		}
		if (strcmp(argv[1], "--version") == 0) {
			std::cout << "RoBoBo-IRC-BoBo Pre-alpha Development Version" << std::endl;
			return 0;
		}
	}
	new ModuleInterface (new ConfigReader ());
	pthread_exit(NULL);
}