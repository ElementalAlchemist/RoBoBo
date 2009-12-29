#include "robobo.h"

int main(int argc, char** argv) {
	ConfigReader config;
	ModuleInterface* modInterface = new ModuleInterface (&config, modInterface);
	pthread_exit(NULL);
}