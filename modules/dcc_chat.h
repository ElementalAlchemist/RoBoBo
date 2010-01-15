class dccChat : public Module {
	public:
		void onDCCReceive(std::string dccid, std::string message);
};

class dccSender : public Module {
	public:
		void dccSend(std::string dccid, std::string message);
};