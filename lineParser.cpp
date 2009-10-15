#include "lineParser.h"

std::vector<std::string> parseLine(std::string message) {
	std::string messageString;
	std::vector<std::string> messageParams;
	messageString = "";
	for (unsigned int i = 0; i < message.size(); i++) {
		if (message[i] == ' ') {
			messageParams.push_back(messageString);
			messageString = "";
		} else if (message[i] == ':' && messageString == "" && i > 0) {
			for (i++; i < message.size(); i++)
				messageString += message[i];
			messageParams.push_back(messageString);
			messageString = "";
		} else
			messageString += message[i];
	}
	return messageParams;
}