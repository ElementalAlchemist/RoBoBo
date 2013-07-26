#include "config.h"

Config::instance = new Config;

static Config* Config::getHandle() {
	return instance;
}

void Config::setMainConfigFile(const std::string& configFileName) {
	if (filename.empty())
		filename = configFileName;
}

void Config::setWorkingDirectory(const std::string& workingDirectory) {
	if (workingDir.empty())
		workingDir = workingDirectory;
}

void Config::readConfig() {
	configData.clear();
	readConfig(filename);
	for (auto callback : notifyList)
		callback();
}

void Config::addRehashNotify(std::function<void()> notifyCallback) {
	notifyList.push_back(notifyCallback);
}

size_t Config::blockCount(const std::string& block) const {
	return configData.count(block);
}

std::list<std::unordered_map<std::string, std::string>> Config::getBlock(const std::string& block) const {
	std::list<std::unordered_map<std::string, std::string>> blockList;
	std::unordered_multimap<std::string, std::unordered_map<std::string, std::string>>::const_iterator startIter, endIter;
	std::tie(startIter, endIter) = configData.equal_range(block);
	for (; startIter != endIter; ++startIter)
		blockList.push_back(startIter->second);
	return blockList;
}

std::string Config::getValue(const std::string& block, const std::string& key) const {
	auto blockIter = configData.find(block);
	if (blockIter == configData.end())
		return "";
	auto dataIter = blockIter->second.find(key);
	if (dataIter == blockIter->second.end())
		return "";
	return dataIter->second;
}

bool Config::getBoolValue(const std::string& block, const std::string& key) const {
	const std::string& value = getValue(block, key);
	std::string lowerValue;
	std::transform(value.begin(), value.end(), std::back_inserter(lowerValue), ::tolower);
	if (lowerValue == "yes" || lowerValue == "on" || lowerValue == "true")
		return true;
	return false;
}

std::list<const std::string&> Config::getAllValues(const std::string& block, const std::string& key) const {
	std::list<const std::string&> valueList;
	std::unordered_multimap<std::string, std::unordered_map<std::string, std::string>>::const_iterator startIter, endIter;
	std::tie(startIter, endIter) = configData.equal_range(block);
	for (; startIter != endIter; ++startIter) {
		auto dataIter = startIter->second.find(key);
		if (dataIter == startIter->second.end())
			valueList.push_back("");
		else
			valueList.push_back(dataIter->second);
	}
	return valueList;
}

std::list<bool> Config::getAllBoolValues(const std::string& block, const std::string& key) const {
	std::list<bool> valueList;
	std::unordered_multimap<std::string, std::unordered_map<std::string, std::string>>::const_iterator startIter, endIter;
	std::tie(startIter, endIter) = configData.equal_range(block);
	for (std::string lowerValue; startIter != endIter; ++startIter) {
		auto dataIter = startIter->second.find(key);
		if (dataIter == startIter->second.end())
			valueList.push_back(false);
		else {
			lowerValue.clear();
			std::transform(dataIter->second.begin(), dataIter->second.end(), std::back_inserter(lowerValue), ::tolower);
			if (lowerValue == "yes" || lowerValue == "on" || lowerValue == "true")
				valueList.push_back(true);
			else
				valueList.push_back(false);
		}
	}
	return valueList;
}

Config::Config() {}

void Config::readConfig(const std::string& fileName) {
	// TODO: a bunch of configuration file reading
}