#include "config.h"

Config* Config::getHandle() {
	static Config instance;
	return &instance;
}

void Config::setMainConfigFile(const std::string& configFileName) {
	if (confname.empty())
		confname = configFileName;
}

void Config::readConfig() {
	config = readConfig(confname, std::ifstream(confname));
	for (auto callback : notifyList)
		callback();
}

enum ConfigState { CONFIG_BLOCK, CONFIG_KEY, CONFIG_VALUE, CONFIG_VALUE_NEXT, CONFIG_VALUE_VAR, CONFIG_VALUE_STR, CONFIG_VALUE_STR_ESCAPED };
// This enum is simply an implementation detail for the configuration parser; hence it is placed here instead of the header file.

std::unordered_multimap<std::string, std::unordered_map<std::string, std::string>> Config::readConfig(const std::string& filename, std::istream&& configData) {
	ConfigState state = CONFIG_BLOCK;
	unsigned int lineNum = 1;
	LogManager* logger = LogManager::getHandle();
	std::string blockName, key, value, valueVar;
	std::unordered_map<std::string, std::string> blockValues;
	std::unordered_multimap<std::string, std::unordered_map<std::string, std::string>> localConfig;
	while (configData.good()) {
		char nextChar = configData.get();
		if (nextChar == std::istream::traits_type::eof())
			break;
		if (nextChar == '\n')
			lineNum++;
		if (state == CONFIG_BLOCK) {
			if (nextChar == '{') {
				if (blockName.empty())
					throw ConfigError (filename, lineNum, "A block without a name was opened.");
				state = CONFIG_KEY;
			} else if (nextChar == '}')
				throw ConfigError (filename, lineNum, "A closing brace could not be mached to an open block.");
			else if (nextChar == '#') {
				while (configData.good() && configData.get() != '\n') {}
				lineNum++;
			} else if (nextChar != ' ' && nextChar != '\t' && nextChar != '\r' && nextChar != '\n')
				blockName += nextChar;
		} else if (state == CONFIG_KEY) {
			if (nextChar == '#') {
				while (configData.good() && configData.get() != '\n') {}
				lineNum++;
			} else if (nextChar == '{')
				throw ConfigError (filename, lineNum, "A block was opened inside another block.");
			else if (nextChar == '}') {
				if (blockValues.empty())
					throw ConfigError (filename, lineNum, "An empty block was defined.  What was the point of that?");
				if (!key.empty() || !value.empty())
					throw ConfigError (filename, lineNum, "A key-value pair was unfinished when the block was terminated.");
				localConfig.insert(std::pair<std::string, std::unordered_map<std::string, std::string>> (blockName, blockValues));
				blockName.clear();
				blockValues.clear();
				logger->log(LOG_ALL, "config", "Block " + blockName + " read.");
				state = CONFIG_BLOCK;
			} else if (nextChar == ';')
				throw ConfigError (filename, lineNum, "A key was specified without a value.");
			else if (nextChar == '=')
				state = CONFIG_VALUE_NEXT;
			else if (nextChar != ' ' && nextChar != '\t' && nextChar != '\r' && nextChar != '\n')
				key += nextChar;
		} else if (state == CONFIG_VALUE) {
			if (nextChar == '#') {
				while (configData.good() && configData.get() != '\n') {}
				lineNum++;
			} else if (nextChar == ';') {
				blockValues.insert(std::pair<std::string, std::string> (key, value));
				key.clear();
				value.clear();
				state = CONFIG_KEY;
			} else if (nextChar == '+')
				state = CONFIG_VALUE_NEXT;
			else
				throw ConfigError (filename, lineNum, "An unexpected character was encountered; expected ';' or '+' (perhaps you forgot to end or properly concatenate your value?).");
		} else if (state == CONFIG_VALUE_NEXT) {
			if (nextChar == '#') {
				while (configData.good() && configData.get() != '\n') {}
				lineNum++;
			} else if (nextChar == '"')
				state = CONFIG_VALUE_STR;
			else if (nextChar != ' ' && nextChar != '\t' && nextChar != '\r' && nextChar != '\n') {
				valueVar += nextChar;
				state = CONFIG_VALUE_VAR;
			}
		} else if (state == CONFIG_VALUE_VAR) {
			if (nextChar == '"')
				throw ConfigError (filename, lineNum, "A string value was specified in the middle of a variable (perhaps you forgot to properly concatenate with '+'?).");
			else if (nextChar == '{')
				throw ConfigError (filename, lineNum, "A block was opened inside another block.");
			else if (nextChar == '}')
				throw ConfigError (filename, lineNum, "A key-value pair was unfinished when the block was terminated.");
			else if (nextChar == ' ' || nextChar == '\t' || nextChar == '\r' || nextChar == '\n' || nextChar == '+' || nextChar == ';') {
				auto keyIter = blockValues.find(valueVar);
				if (keyIter == blockValues.end())
					throw ConfigError (filename, lineNum, "The variable " + valueVar + " was specified, but did not match a variable already specified in the block.");
				value += keyIter->second;
				valueVar.clear();
				if (nextChar == ';') {
					state = CONFIG_KEY;
					blockValues.insert(std::pair<std::string, std::string> (key, value));
					key.clear();
					value.clear();
				} else if (nextChar == '+')
					state = CONFIG_VALUE_NEXT;
				else
					state = CONFIG_VALUE;
			} else
				valueVar += nextChar;
		} else if (state == CONFIG_VALUE_STR) {
			if (nextChar == '\\')
				state = CONFIG_VALUE_STR_ESCAPED;
			else if (nextChar == '"')
				state = CONFIG_VALUE;
			else
				value += nextChar;
		} else if (state == CONFIG_VALUE_STR_ESCAPED) {
			if (nextChar == '\\')
				value += '\\';
			else if (nextChar == 'n')
				value += '\n';
			else if (nextChar == 't')
				value += '\t';
			else if (nextChar == 'r')
				value += '\r';
			else
				value += nextChar;
			state = CONFIG_VALUE_STR;
		}
	}
	if (!blockValues.empty())
		throw ConfigError (filename, lineNum, "A block remained unterminated at the end of the file.");
	std::list<std::unordered_multimap<std::string, std::unordered_map<std::string, std::string>>> includedConfigs;
	for (auto block : localConfig) {
		if (block.first == "include") {
			auto inclfIter = block.second.find("file");
			if (inclfIter != block.second.end()) {
				std::string name = inclfIter->second;
				logger->log(LOG_ALL, "config", "Reading included file " + inclfIter->second);
				includedConfigs.push_back(readConfig(inclfIter->second, std::ifstream(name)));
			} else {
				auto incleIter = block.second.find("executable");
				if (incleIter != block.second.end()) {
					std::string cmd = incleIter->second;
					logger->log(LOG_ALL, "config", "Getting configuration data from `" + incleIter->second + "`");
					FILE* pipe = popen(cmd.c_str(), "r");
					if (!pipe)
						throw ConfigError (cmd, 0, "Could not open pipe to read output.");
					char buffer[256];
					std::stringstream output;
					while (!feof(pipe)) {
						if (fgets(buffer, 256, pipe) != NULL)
							output << buffer;
					}
					pclose(pipe);
					includedConfigs.push_back(readConfig(cmd, std::move(output)));
				} else
					throw ConfigError (filename, lineNum, "An include block was present that contained neither a file nor an executable.");
			}
		}
	}
	for (auto includes : includedConfigs) {
		for (auto conf : includes)
			localConfig.insert(std::pair<std::string, std::unordered_map<std::string, std::string>> (conf.first, conf.second));
	}
	return localConfig;
}

void Config::addRehashNotify(std::function<void()> notifyCallback) {
	notifyList.push_back(notifyCallback);
}

size_t Config::blockCount(const std::string& block) const {
	return config.count(block);
}

std::list<std::unordered_map<std::string, std::string>> Config::getBlock(const std::string& block) const {
	std::list<std::unordered_map<std::string, std::string>> blockList;
	std::unordered_multimap<std::string, std::unordered_map<std::string, std::string>>::const_iterator startIter, endIter;
	std::tie(startIter, endIter) = config.equal_range(block);
	for (; startIter != endIter; ++startIter)
		blockList.push_back(startIter->second);
	return blockList;
}

std::unordered_map<std::string, std::string> Config::getSingleBlock(const std::string& block, const std::unordered_map<std::string, std::string>& conditions) const {
	std::unordered_multimap<std::string, std::unordered_map<std::string, std::string>>::const_iterator startIter, endIter;
	std::tie(startIter, endIter) = config.equal_range(block);
	for (; startIter != endIter; ++startIter) {
		for (auto check : conditions) {
			auto blockIter = startIter->second.find(check.first);
			if (blockIter == startIter->second.end())
				continue;
			if (check.second == blockIter->second)
				return startIter->second;
		}
	}
	return std::unordered_map<std::string, std::string> ();
}

std::string Config::getValue(const std::string& block, const std::string& key) const {
	auto blockIter = config.find(block);
	if (blockIter == config.end())
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

std::list<std::string> Config::getAllValues(const std::string& block, const std::string& key) const {
	std::list<std::string> valueList;
	std::unordered_multimap<std::string, std::unordered_map<std::string, std::string>>::const_iterator startIter, endIter;
	std::tie(startIter, endIter) = config.equal_range(block);
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
	std::tie(startIter, endIter) = config.equal_range(block);
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

bool Config::makeBool(const std::string& value) {
	std::string lowerValue;
	std::transform(value.begin(), value.end(), std::back_inserter(lowerValue), ::tolower);
	if (lowerValue == "yes" || lowerValue == "on" || lowerValue == "true")
		return true;
	return false;
}

Config::Config() {}

ConfigError::ConfigError(const std::string& filename, unsigned int lineNum, const std::string& description) {
	std::ostringstream descStr;
	descStr << "An error occurred reading the configuration from '" << filename << "' on line " << lineNum << ": " << description;
	desc = descStr.str();
}