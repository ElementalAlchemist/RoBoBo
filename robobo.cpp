#include "robobo.h"

int main(int argc, char** argv) {
	ConfigReader config;
	new ModuleInterface (&config);
	pthread_exit(NULL);
}