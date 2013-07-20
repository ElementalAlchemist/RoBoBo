#include "ircmessage.h"

IRCMessage::IRCMessage(const std::string& line) {
	size_t spacePos = line.find(' ');
	if (line[0] == '@') {
		std::string tagLine (line.substr(1, spacePos - 1));
		line = line.substr(spacePos + 1);
		spacePos = line.find(' ');
		size_t sepPos = tagLine.find(';');
		while (!tagLine.empty()) {
			std::string nextTag (tagLine.substr(0, sepPos));
			if (sepPos == std::string::npos)
				tagLine.clear();
			else
				tagLine = tagLine.substr(sepPos + 1);
			sepPos = tagLine.find(';');
			size_t valuePos = nextTag.find('=');
			if (valuePos == std::string::npos)
				linetags.insert(std::pair<std::string, std::string> (nextTag, ""));
			else {
				std::string nextValue (nextTag.substr(valuePos + 1));
				nextTag = nextTag.substr(0, valuePos);
				linetags.insert(std::pair<std::string, std::string> (nextTag, nextValue));
			}
		}
	}
	if (line[0] == ':') {
		lineprefix = line.substr(1, spacePos - 1);
		line = line.substr(spacePos + 1);
		spacePos = line.find(' ');
	}
	linecommand = line.substr(0, spacePos);
	line = line.substr(spacePos + 1);
	spacePos = line.find(' ');
	while (!line.empty()) {
		if (line[0] == ':' || spacePos == std::string::npos) {
			lineparams.push_back(line);
			line.clear();
		} else {
			lineparams.push_back(line.substr(0, spacePos));
			line = line.substr(spacePos + 1);
			spacePos = line.find(' ');
		}
	}
}

std::string IRCMessage::rawLine() const {
	std::string line (linecommand);
	if (!lineparams.empty()) {
		if (lineparams[lineparams.size() - 1].find(' ') == std::string::npos) {
			for (std::string param : lineparams)
				line += " " + param;
		} else {
			for (size_t i = 0; i < lineparams.size() - 1; i++)
				line += " " + lineparams[i];
			line += " :" + lineparams[lineparams.size() - 1];
		}
	}
	if (!lineprefix.empty())
		line = ":" + lineprefix + " " + line;
	return line;
}

const std::string& IRCMessage::command() const {
	return linecommand;
}

const std::vector<std::string>& IRCMessage::params() const {
	return lineparams;
}

const std::string& IRCMessage::prefix() const {
	return lineprefix;
}

std::string IRCMessage::tagValue(const std::string& tag) const {
	auto tagIter = linetags.find(tag);
	if (tagIter == linetags.end())
		return "";
	return tagIter->second;
}

bool IRCMessage::tagExists(const std::string& tag) const {
	return linetags.find(tag) != linetags.end();
}

const std::unordered_map<std::string, std::string>& IRCMessage::tags() const {
	return linetags;
}