#pragma once
#include "main.h"

class IRCMessage {
	public:
		IRCMessage(const std::string& line);
		std::string rawLine() const;
		const std::string& command() const;
		const std::vector<std::string>& params() const;
		const std::string& prefix() const;
		std::string tagValue(const std::string& tag) const;
		bool tagExists() const;
		const std::unordered_map<std::string, std::string>& tags() const;
		void setParam(size_t param, const std::string& value);
		void setParams(const std::vector<std::string>& params);
		void setPrefix(const std::string& prefix);
		void removePrefix();
		void setTag(const std::string& tag, const std::string& value);
		void removeTag(const std::string& tag);
		void setTags(const std::unordered_map<std::string, std::string>& tags);
	private:
		std::string linecommand;
		std::vector<std::string> lineparams;
		std::string lineprefix;
		std::unordered_map<std::string, std::string> linetags;
};