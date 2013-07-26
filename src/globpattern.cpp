#include "globpattern.h"

bool matchGlobPattern(const std::string& pattern, const std::string& target) {
	size_t patternPos = 0, targetPos = 0, patternBack = 0, targetBack = 0;
	if (pattern.empty() && !target.empty())
		return false;
	if (target.empty()) {
		if (pattern.empty() || pattern.find_first_not_of('*') == std::string::npos)
			return true;
		return false;
	}
	while (pattern[patternPos] != '*') {
		if (pattern[patternPos] != target[targetPos] && pattern[patternPos] != '?')
			return false;
		patternPos++;
		targetPos++;
		if (targetPos >= target.size()) {
			if (patternPos >= pattern.size())
				return true;
			return pattern.find_first_not_of('*', patternPos) == std::string::npos;
		}
		if (patternPos >= pattern.size())
			return false;
	}
	while (targetPos < target.size()) {
		if (pattern[patternPos] == '*') {
			if (++patternPos == pattern.size())
				return true;
			patternBack = patternPos;
			targetBack = targetPos + 1;
			continue;
		}
		if (pattern[patternPos] == target[targetPos] || pattern[patternPos] == '?') {
			patternPos++;
			targetPos++;
		} else {
			patternPos = patternBack;
			targetPos = targetBack++;
		}
	}
	return (patternPos == pattern.size() || pattern.find_first_not_of('*', patternPos) == std::string::npos);
}