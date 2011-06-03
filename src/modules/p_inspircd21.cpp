#include "protoinclude.h"

class InspIRCd;
class User {
	public:
		User(std::string server, std::string theNick, std::string theIdent, std::string theHost, std::string theVHost, std::string theGecos, std::string theIP, time_t theConnectTime, std::set<std::string> theUModes);
		std::string server();
		std::string nick();
		std::string ident();
		std::string host();
		std::string vhost();
		std::string ipAddr();
		std::string gecos();
		time_t connectionTime();
		std::string opertype();
		std::set<std::string> modes();
		void addMode(std::string mode);
		void removeMode(std::string mode);
		std::set<char> snomasks();
		void addSnomask(char snomask);
		void removeSnomask(char snomask);
		std::set<std::string> channels();
		void joinChannel(std::string channel);
		void partChannel(std::string channel);
		std::set<std::string> statuses(std::string channel);
		void addStatus(std::string channel, std::string status);
		void removeStatus(std::string channel, std::string status);
	private:
		std::string sid, userNick, userIdent, userHost, vHost, GECOS, ip, oper;
		time_t connectTime;
		std::set<std::string> userModes;
		std::set<char> SNOMasks;
		std::tr1::unordered_map<std::string, std::set<std::string> > inChannels;
};

class Channel {
	public:
		std::set<std::string> modes();
		void addMode(std::string mode);
		void removeMode(std::string mode);
		std::set<std::string> users();
		void joinUser(std::string user);
		void partUser(std::string user);
		void joinUsers(std::set<std::string> users);
		std::string topic();
		void topic(std::string newTopic);
	private:
		std::set<std::string> chanModes, chanUsers;
		std::string chanTopic;
};

class InspIRCd : public Protocol {
	public:
		InspIRCd(std::string serverAddr, std::tr1::unordered_map<std::string, std::string> config, Base* base, unsigned short debug);
		std::list<std::pair<std::string, char> > prefixes();
		std::set<char> channelTypes();
		std::vector<std::vector<std::string> > channelModes();
		std::list<std::string> channels();
		std::string channelTopic(std::string channel);
		std::set<std::string> channelUsers(std::string channel);
		std::string userIdent(std::string user);
		std::string userHost(std::string user);
		std::pair<std::string, char> userStatus(std::string channel, std::string user);
		std::string compareStatus(std::set<std::string> statuses);
		void sendMsg(std::string client, std::string target, std::string message);
		void sendNotice(std::string client, std::string target, std::string message);
		void setMode(std::string client, std::string target, std::string mode);
		void removeMode(std::string client, std::string target, std::string mode);
		void joinChannel(std::string client, std::string channel, std::string key = "");
		void partChannel(std::string client, std::string channel, std::string reason = "");
		void quitServer(std::string client, std::string reason = "");
		void kickUser(std::string client, std::string channel, std::string user, std::string reason = "");
		void changeNick(std::string client, std::string newNick);
		void oper(std::string client, std::string username, std::string password, std::string opertype);
		void sendNumeric(std::string numeric, std::string target, std::vector<std::string> numericData);
		void killUser(std::string client, std::string user, std::string reason);
		void setXLine(std::string client, char lineType, std::string hostmask, std::string time, std::string reason);
		void removeXLine(std::string client, char lineType, std::string hostmask);
		void sendOther(std::string client, std::string rawLine);
		void addClient(std::string nick, std::string ident, std::string host, std::string gecos);
		void removeClient(std::string client);
		std::list<std::string> clients();
		std::tr1::unordered_map<std::string, std::string> clientInfo(std::string client);
		std::list<std::string> userModes(std::string client);
	private:
		pthread_t receiveThread;
		pthread_attr_t detachedState;
		static void* receiveData_thread(void* ptr);
		void receiveData();
};

User::User(std::string theNick, std::string theIdent, std::string theHost, std::string theVHost, std::string theGecos, std::string theIP, time_t theConnectTime, std::set<std::string> theUModes) : sid(server), userNick(theNick), userIdent(theIdent), userHost(theHost), vHost(theVHost), GECOS(theGecos), ip(theIP), connectTime(theConnectTime), userModes(theUModes) {}

std::string User::server() {
	return sid;
}

std::string User::nick() {
	return userNick;
}

std::string User::ident() {
	return userIdent;
}

std::string User::host() {
	return userHost;
}

std::string User::vhost() {
	return vHost;
}

std::string User::ipAddr() {
	return ip;
}

std::string User::gecos() {
	return GECOS;
}

time_t User::connectionTime() {
	return connectTime;
}

std::string User::opertype() {
	return oper;
}

std::set<std::string> User::modes() {
	return userModes;
}

void User::addMode(std::string mode) {
	userModes.insert(mode);
}

void User::removeMode(std::string mode) {
	std::set<std::string>::iterator modeIter = userModes.find(mode);
	if (modeIter != userModes.end())
		userModes.erase(modeIter);
}

std::set<char> User::snomasks() {
	return SNOMasks;
}

void User::addSnomask(char snomask) {
	SNOMasks.insert(snomask);
}

void User::removeSnomask(char snomask) {
	std::set<char>::iterator maskIter = SNOMasks.find(snomask);
	if (maskIter != SNOMasks.end())
		SNOMasks.erase(maskIter);
}

std::set<std::string> User::channels() {
	return inChannels;
}

void User::joinChannel(std::string channel) {
	inChannels.insert(std::pair<std::string, std::set<std::string> (channel, std::set<std::string> ()));
}

void User::partChannel(std::string channel) {
	std::set<std::string>::iterator chanIter = inChannels.find(channels);
	if (chanIter != inChannels.end())
		inChannels.erase(chanIter);
}

std::set<std::string> User::statuses(std::string channel) {
	std::tr1::unordered_map<std::string, std::set<std::string> >::iterator chanIter = inChannels.find(channel);
	if (chanIter == inChannels.end())
		return std::set<std::string> ();
	return chanIter->second;
}

std::set<std::string> Channel::modes() {
	return chanModes;
}

void Channel::addMode(std::string mode) {
	chanModes.insert(mode);
}

void Channel::removeMode(std::string mode) {
	std::set<std::string>::iterator modeIter = chanModes.find(mode);
	if (modeIter != chanModes.end())
		chanModes.erase(modeIter);
}

std::set<std::string> Channel::users() {
	return chanUsers;
}

void Channel::joinUser(std::string user) {
	chanUsers.insert(user);
}

void channel::partUser(std::string user) {
	std::set<std::string>::iterator userIter = chanUsers.find(user);
	if (userIter != chanUsers.end())
		chanUsers.erase(userIter);
}

void Channel::joinUsers(std::set<std::string> users) {
	chanUsers.insert(users.begin(), users.end());
}

std::string Channel::topic() {
	return chanTopic;
}

void Channel::topic(std::string newTopic) {
	chanTopic = newTopic;
}

InspIRCd::InspIRCd(std::string serverAddr, std::tr1::unordered_map<std::string, std::string> config, Base* base, unsigned short debug) : Protocol(serverAddr, config, base, debug) {}

std::list<std::pair<std::string, char> > InspIRCd::prefixes() {
	
}

std::set<char> InspIRCd::channelTypes() {
	std::set<char> chanTypes;
	chanTypes.insert('#');
	return chanTypes;
}

std::vector<std::vector<std::string> > InspIRCd::channelModes() {
	
}

std::list<std::string> InspIRCd::channels() {
	
}

std::string InspIRCd::channelTopic(std::string channel) {
	
}

std::set<std::string> InspIRCd::channelUsers(std::string channel) {
	
}

std::string InspIRCd::userIdent(std::string user) {
	
}

std::string InspIRCd::userHost(std::string user) {
	
}

std::pair<std::string, char> InspIRCd::userStatus(std::string channel, std::string user) {
	
}

std::string InspIRCd::compareStatus(std::set<std::string> statuses) {
	
}

void InspIRCd::sendMsg(std::string client, std::string target, std::string message) {
	
}

void InspIRCD::sendNotice(std::string client, std::string target, std::string message) {
	
}

void InspIRCd::setMode(std::string client, std::string target, std::string mode) {
	
}

void InspIRCd::removeMode(std::string client, std::string target, std::string mode) {
	
}

void InspIRCd::joinChannel(std::string client, std::string channel, std::string key) {
	
}

void InspIRCd::partChannel(std::string client, std::string channel, std::string reason) {
	
}

void InspIRCd::quitServer(std::string client, std::string reason) {
	
}

void InspIRCD::kickUser(std::string client, std::string channel, std::string user, std::string reason) {
	
}

void InspIRCd::changeNick(std::string client, std::string newNick) {
	
}

void InspIRCd::oper(std::string client, std::string username, std::string password, std::string opertype) {
	
}

void InspIRCd::sendNumeric(std::string numeric, std::string target, std::vector<std::string> numericData) {
	
}

void InspIRCd::killUser(std::string client, std::string user, std::string reason) {
	
}

void InspIRCd::setXLine(std::string client, char lineType, std::string hostmask, std::string time, std::string reason) {
	
}

void InspIRCd::removeXLine(std::string client, char lineType, std::string hostmask) {
	
}

void InspIRCd::sendOther(std::string client, std::string rawLine) {
	
}

void InspIRCd::addClient(std::string nick, std::string ident, std::string host, std::string gecos) {
	
}

void InspIRCd::removeClient(std::string client) {
	
}

std::list<std::string> InspIRCd::clients() {
	
}

std::tr1::unordered_map<std::string, std::string> InspIRCd::clientInfo(std::string client) {
	
}

std::list<std::string> InspIRCd::userModes(std::string client) {
	
}

static void* InspIRCd::receiveData_thread(void* ptr) {
	InspIRCd* protoClass = (InspIRCd*) ptr;
	protoClass->receiveData();
	return NULL;
}

void InspIRCd::receiveData() {
	
}

extern "C" Protocol* spawn(std::string serverAddr, std::tr1::unordered_map<std::string, std::string> config, Base* base, unsigned short debugLevel) {
	return new InspIRCd (serverAddr, config, base, debugLevel);
}