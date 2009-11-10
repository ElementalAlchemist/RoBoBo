#include "config.h"

#ifndef CONFIG_ROBOBO
#define CONFIG_ROBOBO
class ConfigReader {
	public:
		ConfigReader();
		ConfigReader(std::string filename);
		void readConfig(std::string filename);
		std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > getServerConfig();
		std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > getModConfig();
	private:
 		std::ifstream configFile;
		std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > serverConfig;
		std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > modConfig;
};

ConfigReader::ConfigReader() {
	readConfig("robobo.conf");
}

ConfigReader::ConfigReader(std::string filename) {
	readConfig(filename);
}

void ConfigReader::readConfig(std::string filename) {
	configFile.open(filename.c_str());
	std::string configuration;
	configFile >> configuration;
	configFile.close();
	int lineNumber = 1;
	std::string sectionType = "", sectionName = "", varName = "", currentValue = "", concatingVar = "";
	bool inBlock = false, typingSection = false, namingSection = false, escaped = false, escapedNow = false, commentable = true, writing = false, acceptVar = false, concatable = false, concatening = false;
	std::vector<std::string> includes;
	std::tr1::unordered_map<std::string, std::string> oneBlock;
	for (unsigned int i = 0; i < configuration.size(); i++) {
		if (configuration[i] == '\n')
			lineNumber++;
		
		if (!inBlock && sectionType == "")
			typingSection = true;
		else if (typingSection) {
			for ( ; configuration[i] != ' '; i++)
				sectionType += configuration[i];
			typingSection = false;
			namingSection = true;
		} else if (namingSection) {
			for ( ; configuration[i] != ' ' && configuration[i] != '{'; i++)
				sectionName += configuration[i];
			namingSection = false;
		} else if (configuration[i] == '{') {
			inBlock = true;
			acceptVar = true;
		} else if (configuration[i] == '}') {
			if (!inBlock) {
				if (sectionType == "include")
					readConfig(sectionName);
				else {
					std::perror("An end brace occurred outside a block before a corresponding opening brace existed in the configuration file on line " + lineNumber);
					std::exit(0);
				}
			}
			inBlock = false;
			if (sectionType == "server") {
				serverConfig.insert(std::pair<std::string, std::tr1::unordered_map<std::string, std::string> > (sectionName, oneBlock));
				oneBlock.clear();
			} else if (sectionType == "module") {
				modConfig.insert(std::pair<std::string, std::tr1::unordered_map<std::string, std::string> > (sectionName, oneBlock));
				oneBlock.clear();
			} else {
				std::perror("An invalid block type was declared in the configuration file.  This block ends on line " + lineNumber);
				std::exit(0);
			}
		} else if (configuration[i] == '\\' && !escaped)
			escaped = escapedNow = true;
		else if (escaped && configuration[i] == '"')
			currentValue += "\"";
		else if (!escaped && configuration[i] == '"') {
			if (writing) {
				writing = false;
				commentable = true;
				concatable = true;
			} else {
				currentValue += configuration[i];
				writing = true;
				commentable = false;
				concatable = false;
			}
		} else if (writing) {
			currentValue += configuration[i];
		} else if (configuration[i] == '=')
			acceptVar = false;
		else if (configuration[i] == ' ' || configuration[i] == '\t' || configuration[i] == '\r' || configuration[i] == '\n') {
			// ignore whitespace that's not part of a string
		} else if (!escaped && !writing && configuration[i] == ';') { // parse the end of a statement
			oneBlock.insert(std::pair<std::string, std::string> (varName, currentValue));
			varName = "";
			currentValue = "";
			acceptVar = true;
			concatable = false;
		} else if (commentable && configuration[i] == '#') {
			for ( ; configuration[i] != '\n'; i++) {} // ignore the rest of the line
		} else if (acceptVar)
			varName += configuration[i];
		else if (concatable && configuration[i] == '+') {
			concatable = false;
			concatening = true;
		} else if (concatening && configuration[i] == '+') {
			concatening = false;
			concatable = true;
		} else if (concatening)
			concatingVar += configuration[i];
		
		if (!escapedNow && escaped) {
			escaped = false;
		}
 		escapedNow = false;
	}
}

std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > ConfigReader::getServerConfig() {
	return serverConfig;
}

std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > ConfigReader::getModConfig() {
	return modConfig;
}
#endif