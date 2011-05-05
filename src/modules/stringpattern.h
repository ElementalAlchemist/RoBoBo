class PatternMatcher : public Module {
	public:
		virtual ~PatternMatcher();
		virtual bool match(std::string pattern, std::string target);
};

bool PatternMatcher::match(std::string target, std::string pattern) { return false; }