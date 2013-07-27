#pragma once
#include "main.h"
#include "logmanager.h"

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
		std::string confname;
		std::string workingDir;
		std::unordered_multimap<std::string, std::unordered_map<std::string, std::string>> configData;
		std::unordered_multimap<std::string, std::unordered_map<std::string, std::string>> readConfig(const std::string& filename, std::istream&& configData);
		std::list<std::function<void()>> notifyList;
};

class ConfigError : public std::exception {
	public:
		ConfigError(const std::string& fileName, unsigned int lineNum, const std::string& description);
		const char* what() const noexcept { return desc.c_str(); }
	private:
		std::string desc;
};