class dccChat : public Module {
	public:
		virtual void onDCCReceive(std::string dccid, std::string message);
};

class dccSender : public Module {
	public:
		virtual void dccSend(std::string dccid, std::string message);
		virtual bool hookDCCMessage(std::string modName, std::string hookMsg);
};