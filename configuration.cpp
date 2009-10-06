#include "configuration.h"

class ConfigReader {
	public:
		ConfigReader();
		std::string getServer();
		unsigned short getPort();
	private:
		std::string server;
		unsigned short port;
};

ConfigReader::ConfigReader() {
	// This will read the configuration file, in the future
	server = "128.237.157.136";
	port = 6667;
}

std::string ConfigReader::getServer() {
	return server;
}

unsigned short ConfigReader::getPort() {
	return port;
}