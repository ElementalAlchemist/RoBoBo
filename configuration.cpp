#include "configuration.h"

class ConfigReader {
	public:
		ConfigReader();
		std::string getNick();
		std::string getIdent();
		std::string getServer();
		unsigned short getPort();
	private:
		std::string nick, ident;
		std::string server;
		unsigned short port;
};

ConfigReader::ConfigReader() {
	// This will read the configuration file in the future
	nick = "RoBoBo";
	ident = "RoBoBo";
	server = "128.237.157.136";
	port = 6667;
}

std::string ConfigReader::getNick() {
	return nick;
}

std::string ConfigReader::getIdent() {
	return ident;
}

std::string ConfigReader::getServer() {
	return server;
}

unsigned short ConfigReader::getPort() {
	return port;
}