class PatternMatcher : public Module {
	public:
		PatternMatcher(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug);
		virtual ~PatternMatcher();
		virtual bool match(std::string pattern, std::string target);
};

PatternMatcher::PatternMatcher(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug) : Module(modConf, modFace, modName, dir, debug) {}

PatternMatcher::~PatternMatcher() {}

bool PatternMatcher::match(std::string target, std::string pattern) { return false; }