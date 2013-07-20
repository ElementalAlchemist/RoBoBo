#pragma once
#include "main.h"

class Config {
	public:
		static Config* getHandle();
		void setMainConfigFile(const std::string& configFileName);
		void setWorkingDirectory(const std::string& workingDirectory);
		void readConfig();
		size_t blockCount(const std::string& block) const;
		std::list<std::unordered_map<std::string, std::string>> getBlock(const std::string& block) const;
		const std::string& getValue(const std::string& block, const std::string& key) const;
		bool getBoolValue(const std::string& block, const std::string& key) const;
		std::list<const std::string&> getAllValues(const std::string& block, const std::string& key) const;
		std::list<bool> getAllBoolValues(const std::string& block, const std::string& key) const;
	private:
		Config();
		const Config* instance = new Config;
		const std::string filename;
		const std::string workingDir;
		std::unordered_multimap<std::string, std::unordered_map<std::string, std::string>> configData;
		void readConfig(const std::string& fileName);
};