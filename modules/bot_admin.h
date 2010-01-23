class AdminHook : public Module {
	public:
		virtual ~AdminHook();
		virtual std::vector<std::vector<std::string> > adminCommands();
		virtual void onAdminCommand(std::string server, std::string nick, std::string command, std::string message, bool master);
};