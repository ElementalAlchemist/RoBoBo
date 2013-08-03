#include "logmanager.h"

LogManager* LogManager::getHandle() {
	static LogManager instance;
	return &instance;
}

void LogManager::setDefaultLevel(LogLevel level) {
	defaultLog = level;
}

void LogManager::updateLogFiles() {
	for (auto logFileList : logFiles) {
		for (auto file : logFileList.second)
			delete file.second;
	}
	logFiles.clear();
	Config* conf = Config::getHandle();
	std::list<std::unordered_map<std::string, std::string>> logBlockList = conf->getBlocks("log");
	for (std::unordered_map<std::string, std::string> blockContents : logBlockList) {
		if (blockContents["level"] == "none" || blockContents["types"].empty() || blockContents["file"].empty())
			continue;
		LogLevel level = LOG_DEFAULT;
		if (blockContents["level"] == "error")
			level = LOG_ERROR;
		else if (blockContents["level"] == "debug")
			level = LOG_DEBUG;
		else if (blockContents["level"] == "all")
			level = LOG_ALL;
		std::list<std::string> types;
		std::string typeStr = blockContents["types"];
		while (!typeStr.empty()) {
			std::string nextType = typeStr.substr(0, typeStr.find(' '));
			if (typeStr == nextType)
				typeStr.clear();
			else
				typeStr = typeStr.substr(nextType.size() + 1);
			types.push_back(nextType);
		}
		logFiles[level].push_back(std::pair<std::list<std::string>, std::ofstream*> (types, new std::ofstream("logs/" + blockContents["file"], std::ios_base::out | std::ios_base::app)));
	}
}

void LogManager::log(LogLevel level, const std::string& type, const std::string& info) {
	if (level == LOG_NONE)
		return;
	if (level <= defaultLog)
		std::cout << type << ": " << info << std::endl;
	for (int lvl = LOG_ERROR; lvl <= level; lvl++) {
		for (auto logFile : logFiles[static_cast<LogLevel>(lvl)]) {
			bool canWriteType = false;
			for (std::string typeSpec : logFile.first) {
				if (typeSpec[0] == '-')
					canWriteType = !matchGlobPattern(typeSpec.substr(1), type);
				else
					canWriteType = matchGlobPattern(typeSpec, type);
			}
			if (!canWriteType)
				continue;
			std::time_t currTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			std::tm* currTimeStruct = std::gmtime(&currTime);
			// Compilers do not yet support std::put_time, so I must make do with doing it manually for now.
			*(logFile.second) << "[" << (currTimeStruct->tm_year + 1900) << "-" << std::setfill('0') << std::setw(2) << (currTimeStruct->tm_mon + 1) << "-" << std::setw(2) << currTimeStruct->tm_mday << " " << currTimeStruct->tm_hour << ":" << std::setw(2) << currTimeStruct->tm_min << ":" << std::setw(2) << currTimeStruct->tm_sec << "] " << type << ": " << info << std::endl;
		}
	}
}

LogManager::LogManager() {}