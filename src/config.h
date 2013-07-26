#pragma once
#include "main.h"

class Config {
	public:
		// The default constructor is private so that the class can only be used from the handle specified within
		Config(const Config&) = delete; // We make copying and assignment forbidden
		Config& operator=(const Config&) = delete;
		static Config* getHandle();
		void setMainConfigFile(const std::string& configFileName);
		void setWorkingDirectory(const std::string& workingDirectory);
		void readConfig();
		void addRehashNotify(std::function<void()> notifyCallback);
		size_t blockCount(const std::string& block) const;
		std::list<std::unordered_map<std::string, std::string>> getBlock(const std::string& block) const;
		std::string getValue(const std::string& block, const std::string& key) const;
		bool getBoolValue(const std::string& block, const std::string& key) const;
		std::list<std::string> getAllValues(const std::string& block, const std::string& key) const;
		std::list<bool> getAllBoolValues(const std::string& block, const std::string& key) const;
	private:
		Config();
		static const Config* instance;
		std::string filename;
		std::string workingDir;
		std::unordered_multimap<std::string, std::unordered_map<std::string, std::string>> configData;
		void readConfig(const std::string& fileName);
		std::list<std::function<void()>> notifyList;
};