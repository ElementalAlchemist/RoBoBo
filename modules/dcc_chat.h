class dccChat : public Module {
	public:
		virtual ~dccChat();
		virtual void onDCCReceive(std::string dccid, std::string message);
		virtual void onDCCEnd(std::string dccid);
};

class dccSender : public Module {
	public:
		virtual ~dccSender();
		virtual void dccSend(std::string dccid, std::string message);
		virtual bool hookDCCMessage(std::string modName, std::string hookMsg);
		virtual void unhookDCCSession(std::string modName, std::string dccid);
};

dccChat::~dccChat() {}

void dccChat::onDCCReceive(std::string dccid, std::string message) {}

void dccChat::onDCCEnd(std::string dccid) {}

dccSender::~dccSender() {}

void dccSender::dccSend(std::string dccid, std::string message) {}

bool dccSender::hookDCCMessage(std::string modName, std::string hookMsg) { return false; }

void dccSender::unhookDCCSession(std::string modName, std::string dccid) {}