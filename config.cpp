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
	if (configFile.fail()) {
		std::perror("Config file does not exist or could not be opened");
		std::exit(0);
	}
	char configuration;
	int lineNumber = 1;
	std::string sectionType = "", sectionName = "", varName = "", currentValue = "", concatingVar = "";
	bool inBlock = false, typingSection = false, namingSection = false, escaped = false, escapedNow = false, commentable = true, writing = false, acceptVar = false, concatable = false, concatening = false;
	std::vector<std::string> includes;
	std::tr1::unordered_map<std::string, std::string> oneBlock;
	while (configFile.good()) {
		configuration = configFile.get();
		if (configuration == '\n')
			lineNumber++;
		
		if (!inBlock && sectionType == "")
			typingSection = true;
		
		if (commentable && configuration == '#') {
			while (configuration != '\n' && configFile.good()) {
				configuration = configFile.get(); // do nothing with it--ignore the line
			}
			lineNumber++; // count it as a line, since the \n won't reach the top of the loop where the line number increments
		} else if ((configuration == ' ' || configuration == '\t' || configuration == '\r' || configuration == '\n') && !writing) {
			// ignore whitespace that's not part of a string
		} else if (typingSection) {
			while (configuration != ' ' && configFile.good()) {
				sectionType += configuration;
				configuration = configFile.get();
			}
			if (!configFile.good()) {
				std::ostringstream lineSS;
				lineSS << lineNumber;
				std::string message = "An error occurred reading a section type name in the configuration file.  This error occurred on line " + lineSS.str();
				std::perror(message.c_str());
				std::exit(0);
			}
			typingSection = false;
			namingSection = true;
		} else if (namingSection) {
			while (configuration != ' ' && configuration != '{' && configFile.good()) {
				sectionName += configuration;
				configuration = configFile.get();
			}
			if (!configFile.good()) {
				std::ostringstream lineSS;
				lineSS << lineNumber;
				std::string message = "An error occurred reading a section name from the configuration file.  This error occurred on line " + lineSS.str();
				std::perror(message.c_str());
				std::exit(0);
			}
			namingSection = false;
			if (configuration == '{') { // handle this now since next iteration will handle the next character
				inBlock = true;
				acceptVar = true;
			}
		} else if (configuration == '{') {
			inBlock = true;
			acceptVar = true;
		} else if (configuration == '}') {
			if (!inBlock) {
				std::ostringstream lineSS;
				lineSS << lineNumber;
				std::string message = "An end brace occurred outside a block before a corresponding opening brace existed in the configuration file.  The offending brace can be found on line " + lineSS.str();
				std::perror(message.c_str());
				std::exit(0);
			}
			inBlock = false;
			if (sectionType == "server") {
				serverConfig.insert(std::pair<std::string, std::tr1::unordered_map<std::string, std::string> > (sectionName, oneBlock));
				oneBlock.clear();
			} else if (sectionType == "module") {
				modConfig.insert(std::pair<std::string, std::tr1::unordered_map<std::string, std::string> > (sectionName, oneBlock));
				oneBlock.clear();
			} else {
				std::ostringstream lineSS;
				lineSS << lineNumber;
				std::string message = "An invalid block type was declared in the configuration file.  This block is of type " + sectionType + " and ends on line " + lineSS.str();
				std::perror(message.c_str());
				std::exit(0);
			}
		} else if (configuration == '\\' && !escaped)
			escaped = escapedNow = true;
		else if (escaped && configuration == '"')
			currentValue += "\"";
		else if (!escaped && configuration == '"') {
			if (writing) {
				writing = false;
				commentable = true;
				concatable = true;
			} else {
				currentValue += configuration;
				writing = true;
				commentable = false;
				concatable = false;
			}
		} else if (writing) {
			currentValue += configuration;
		} else if (configuration == '=')
			acceptVar = false;
		else if (!escaped && !writing && configuration == ';') { // parse the end of a statement
			if (!inBlock)
				if (sectionType == "include")
					includes.push_back(sectionName);
			oneBlock.insert(std::pair<std::string, std::string> (varName, currentValue));
			varName = "";
			currentValue = "";
			acceptVar = true;
			concatable = false;
		} else if (acceptVar)
			varName += configuration;
		else if (concatable && configuration == '+') {
			concatable = false;
			concatening = true;
		} else if (concatening && configuration == '+') {
			concatening = false;
			concatable = true;
			// handle concatingVar sometime
		} else if (concatening)
			concatingVar += configuration;
		
		if (!escapedNow && escaped) {
			escaped = false;
		}
 		escapedNow = false;
	}
	configFile.close();
	for (unsigned int i = 0; i < includes.size(); i++)
		readConfig(includes[i]);
}

std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > ConfigReader::getServerConfig() {
	return serverConfig;
}

std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > ConfigReader::getModConfig() {
	return modConfig;
}
#endif