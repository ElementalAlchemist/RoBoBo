#pragma once
#include "main.h"
#include "globpattern.h"
#include "config.h"

enum LogLevel { LOG_NONE = 0, LOG_ERROR = 1, LOG_DEFAULT = 2, LOG_DEBUG = 3, LOG_ALL = 4 };

class LogManager {
	public:
		LogManager(const LogManager&) = delete;
		LogManager& operator=(const LogManager&) = delete;
		static LogManager* getHandle();
		void setDefaultLevel(LogLevel level);
		void updateLogFiles();
		void log(LogLevel level, const std::string& type, const std::string& info);
	private:
		LogManager();
		static const LogManager* instance;
		LogLevel defaultLog;
		std::unordered_map<LogLevel, std::list<std::pair<std::list<std::string>, std::ofstream*>>, std::hash<int>> logFiles;
};