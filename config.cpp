#include "config.h"

#ifndef CONFIG_ROBOBO
#define CONFIG_ROBOBO
class ConfigReader {
	public:
		ConfigReader();
		ConfigReader(std::string filename);
		std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > serverConfig();
		std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > modConfig();
	private:
		std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > serverConfig;
		std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > modConfig;
};

ConfigReader::ConfigReader() {
	ConfigReader("robobo.conf");
}

ConfigReader::ConfigReader(std::string filename) {
	// This will read the config file in the future.
}