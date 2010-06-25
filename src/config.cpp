#include "config.h"

ConfigReader::ConfigReader(std::string filename, std::string filedir) {
	readConfig(filename, filedir);
}

void ConfigReader::readConfig(std::string filename, std::string filedir) {
	std::ifstream configFile;
	std::string absConfFile = filedir + "/" + filename;
	configFile.open(absConfFile.c_str());
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
			lineNumber++; // count it as a line, since the \n won't reach the part of the loop where the line number increments
		} else if ((configuration == ' ' || configuration == '\t' || configuration == '\r' || configuration == '\n') && !writing) {
			// ignore whitespace that's not part of a string
		} else if (typingSection) {
			while (configuration != ' ' && configuration != '\t' && configuration != '\r' && configuration != '\n' && configFile.good()) {
				sectionType += configuration;
				configuration = configFile.get();
			}
			typingSection = false;
			namingSection = true;
		} else if (namingSection) {
			while (configuration != ' ' && configuration != '\t' && configuration != '\r' && configuration != '\n' && configuration != '{' && configuration != ';' && configFile.good()) {
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
			if (configuration == ';' && sectionType == "include")
				includes.push_back(sectionName);
		} else if (configuration == '{') {
			inBlock = true;
			acceptVar = true;
		} else if (configuration == '}' && !writing) {
			if (!inBlock) {
				std::ostringstream lineSS;
				lineSS << lineNumber;
				std::string message = "An end brace occurred outside a block before a corresponding opening brace existed in the configuration file.  The offending brace can be found on line " + lineSS.str();
				std::perror(message.c_str());
				std::exit(0);
			}
			inBlock = false;
			typingSection = true;
			if (sectionType == "server")
				serverConfig.insert(std::pair<std::string, std::tr1::unordered_map<std::string, std::string> > (sectionName, oneBlock));
			else if (sectionType == "serverconf")
				serverKeepConfig.insert(std::pair<std::string, std::tr1::unordered_map<std::string, std::string> > (sectionName, oneBlock));
			else if (sectionType == "module")
				modLoadConfig.insert(std::pair<std::string, std::tr1::unordered_map<std::string, std::string> > (sectionName, oneBlock));
			else if (sectionType == "moduleconf")
				modKeepConfig.insert(std::pair<std::string, std::tr1::unordered_map<std::string, std::string> > (sectionName, oneBlock));
			else {
				std::ostringstream lineSS;
				lineSS << lineNumber;
				std::string message = "An invalid block type was declared in the configuration file.  This block is of type " + sectionType + " and ends on line " + lineSS.str();
				std::perror(message.c_str());
				std::exit(0);
			}
			sectionType = "";
			sectionName = "";
			oneBlock.clear();
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
				writing = true;
				commentable = false;
				concatable = false;
				concatening = false;
			}
		} else if (writing) {
			currentValue += configuration;
		} else if (configuration == '=') {
			acceptVar = false;
			concatening = true;
		} else if (!escaped && !writing && configuration == ';') { // parse the end of a statement
			if (!inBlock) {
				if (sectionType == "include")
					includes.push_back(sectionName);
				else {
					std::ostringstream lineSS;
					lineSS << lineNumber;
					std::string message = "An invalid semicolon was found in the configuration file on line " + lineSS.str();
					std::perror(message.c_str());
					std::exit(0);
				}
			} else {
				if (concatening)
					currentValue += oneBlock[concatingVar];
				oneBlock.insert(std::pair<std::string, std::string> (varName, currentValue));
			}
			varName = "";
			currentValue = "";
			concatingVar = "";
			acceptVar = true;
			concatable = false;
			concatening = false;
		} else if (acceptVar)
			varName += configuration;
		else if (concatable && configuration == '+') {
			concatable = false;
			concatening = true;
		} else if (concatening && configuration == '+') {
			concatening = false;
			concatable = true;
			currentValue += oneBlock[concatingVar];
			concatingVar = "";
		} else if (concatening)
			concatingVar += configuration;
		
		if (!escapedNow && escaped) {
			escaped = false;
		}
 		escapedNow = false;
	}
	configFile.close();
	for (unsigned int i = 0; i < includes.size(); i++)
		readConfig(includes[i], filedir);
}

std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > ConfigReader::getServerConfig(bool connecting) {
	if (connecting)
		return serverConfig;
	return serverKeepConfig;
}

std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > ConfigReader::getModConfig(bool loading) {
	if (loading)
		return modLoadConfig;
	return modKeepConfig;
}