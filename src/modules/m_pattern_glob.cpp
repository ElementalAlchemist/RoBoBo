#include "modinclude.h"
#include "stringpattern.h"

class GlobMatcher : public PatternMatcher {
	public:
		GlobMatcher(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug);
		int botAPIversion();
		std::string description();
		std::vector<std::string> abilities();
		bool match(std::string target, std::string pattern);
};

GlobMatcher::GlobMatcher(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug) : PatternMatcher(modConf, modFace, modName, dir, debug) {}

int GlobMatcher::botAPIversion() {
	return 2001;
}

std::string GlobMatcher::description() {
	return "Allows other modules to test matches to glob patterns.";
}

std::vector<std::string> GlobMatcher::abilities() {
	std::vector<std::string> services;
	services.push_back("PATTERN_GLOB");
	return services;
}

bool GlobMatcher::match(std::string target, std::string pattern) {
	size_t targetPos = 0, patternPos = 0, tPosBack, pPosBack;
	while (targetPos < target.size() && pattern[patternPos] != '*') {
		if (target[targetPos] != pattern[patternPos] && pattern[patternPos] != '?')
			return false;
		targetPos++;
		patternPos++;
	}
	while (targetPos < target.size()) {
		if (pattern[patternPos] == '*') {
			if (++patternPos == pattern.size())
				return true;
			pPosBack = patternPos;
			tPosBack = targetPos + 1;
		} else {
			if (target[targetPos] == pattern[patternPos] || pattern[patternPos] == '?') {
				targetPos++;
				patternPos++;
			} else {
				targetPos = tPosBack++;
				patternPos = pPosBack;
			}
		}
	}
	while (pattern[patternPos] == '*')
		patternPos++;
	return (patternPos == pattern.size());
}

MODULE_SPAWN(GlobMatcher)