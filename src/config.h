#ifndef CONFIG_H
#define CONFIG_H
#include "main.h"

#include <fstream>
#include <sstream>

class ConfigReader {
	public:
		ConfigReader(std::string filename, std::string filedir);
		void readConfig(std::string filename, std::string filedir);
		std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > getServerConfig(bool connecting);
		std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > getModConfig(bool loading);
	private:
		std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > serverConfig;
		std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > serverKeepConfig;
		std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > modLoadConfig;
		std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > modKeepConfig;
};

#endif