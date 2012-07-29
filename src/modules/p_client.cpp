#include "protocol.h"

class User {
	public:
		User(std::string theNick, std::string theIdent, std::string theHost);
		std::string nick;
		std::string ident;
		std::string host;
		std::set<std::string> channels;
};

class Channel {
	public:
		std::string topic;
		std::list<std::string> modes;
		std::unordered_map<std::string, std::list<std::string>> listModes;
		std::set<std::string> users;
		std::unordered_map<char, std::set<std::string>> statuses;
};

class LocalClient : public User {
	public:
		LocalClient(std::string theNick, std::string theIdent, std::string theHost, std::string theGecos);
		std::string gecos;
		std::set<std::string> modes;
};

User::User(std::string theNick, std::string theIdent, std::string theHost) : nick(theNick), ident(theIdent), host(theHost) {}

LocalClient::LocalClient(std::string theNick, std::string theIdent, std::string theHost, std::string theGecos) : User(theNick, theIdent, theHost), gecos(theGecos) {}

class Client : public Protocol {
	public:
		Client(std::string server, std::map<std::string, std::string> conf, std::string workDir, bool dumpLogs, unsigned short debug, Base* botptr);
		unsigned int apiVersion() = 0;
		void connectServer();
		void disconnectServer(std::string reason);
		bool isConnected();
		bool deadServer();
		bool isClient();
		
		void sendPrivMsg(const std::string& client, const std::string& target, const std::string& message);
		void sendNotice(const std::string& client, const std::string& target, const std::string& message);
		void sendCTCP(const std::string& client, const std::string& target, const std::string& ctcp, const std::string& params);
		void sendCTCPReply(const std::string& client, const std::string& target, const std::string& ctcp, const std::string& params);
		void setMode(const std::string& client, const std::string& target, const std::list<std::string>& setModes, const std::list<std::string>& remModes);
		void setSNOmask(const std::string& client, bool add, char snomask);
		void joinChan(const std::string& client, const std::string& channel, const std::string& key);
		void partChan( const std::string& client, const std::string& channel, const std::string& reason);
		void kickUser(const std::string& client, const std::string& channel, const std::string& user, const std::string& reason);
		std::string addClient(std::string& nick, std::string& ident, std::string& host, std::string& gecos);
		void removeClient(const std::string& client);
		void setTopic(const std::string& client, const std::string& channel, const std::string& topic);
		void inviteUser(const std::string& client, const std::string& channel, const std::string& user);
		void knockOnChannel(const std::string& client, const std::string& channel, const std::string& reason);
		void changeNick(const std::string& user, const std::string& newNick);
		void sendPing(const std::string& remoteServer);
		void operUp(const std::string& client, const std::string& usernameOrType, const std::string& password);
		void sendServerNotice(char snomask, const std::string& message);
		void setMetadata(const std::string& target, const std::string& key, const std::string& value);
		void setXLine(const std::string& client, const std::string& lineType, const std::string& mask, time_t duration, const std::string& reason);
		void remXLine(const std::string& client, const std::string& lineType, const std::string& mask);
		void changeIdent(const std::string& user, const std::string& newIdent);
		void changeHost(const std::string& user, const std::string& newHost);
		void changeGecos(const std::string& user, const std::string& newGecos);
		void sendWallops(const std::string& client, const std::string& message);
		void sendOtherData(const std::string& client, const std::string& line);
		
		std::list<std::string> networkServerList();
		std::list<std::string> xLineTypes();
		std::list<std::string> xLineList(const std::string& lineType);
		time_t xLineExpiry(const std::string& lineType, const std::string& mask);
		std::string xLineReason(const std::string& lineType, const std::string& mask);
		std::list<std::string> chanListModes();
		std::list<std::string> chanParamModes();
		std::list<std::string> chanNoParamModes();
		std::list<std::pair<std::string, char>> chanPrefixes();
		std::pair<std::string, char> compareStatus(const std::string& status0, const std::string& status1);
		std::pair<std::string, char> compareStatus(const std::string& status0, char status1);
		std::pair<std::string, char> compareStatus(char status0, char status1);
		
		std::string chanTopic(const std::string& channel);
		time_t chanTimestamp(const std::string& channel);
		std::set<std::string> chanUsers(const std::string& channel);
		bool userInChan(const std::string& channel, const std::string& user);
		std::pair<std::string, char> userStatus(const std::string& channel, const std::string& user);
		bool userHasStatus(const std::string& channel, const std::string& user, const std::string& status);
		bool userHasStatus(const std::string& channel, const std::string& user, char status);
		bool userHasStatusOrGreater(const std::string& channel, const std::string& user, const std::string& status);
		bool userHasStatusOrGreater(const std::string& channel, const std::string& user, char status);
		std::list<std::string> chanModes(const std::string& channel);
		std::list<std::string> chanListModeList(const std::string& channel, const std::string& listMode);
		bool chanHasMode(const std::string& channel, const std::string& mode);
		std::string chanModeParam(const std::string& channel, const std::string& mode);
		
		std::list<std::string> clientList();
		std::string clientNick(const std::string& client);
		std::string userIdent(const std::string& user);
		std::string userHost(const std::string& user);
		std::string userGecos(const std::string& user);
		std::set<std::string> userModes(const std::string& user);
		bool userHasMode(const std::string& user, const std::string& mode);
		std::set<char> userSNOmasks(const std::string& user);
		bool userHasSNOmask(const std::string& user, char snomask);
		std::set<std::string> userChans(const std::string& user);
		time_t userTimestamp(const std::string& user);
		time_t userNickTimestamp(const std::string& user);
		
		void processedOutChanMsg(const std::string& client, const std::string& channel, char status, const std::string& message);
		void processedOutUserMsg(const std::string& client, const std::string& nick, const std::string& message);
		void processedOutChanNotice(const std::string& client, const std::string& channel, char status, const std::string& message);
		void processedOutUserNotice(const std::string& client, const std::string& nick, const std::string& message);
		void processedOutChanCTCP(const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params);
		void processedOutUserCTCP(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params);
		void processedOutChanCTCPReply(const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params);
		void processedOutUserCTCPReply(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params);
	private:
		
};

Client::Protocol(std::string server, std::map<std::string, std::string> conf, std::string workDir, bool dumpLogs, unsigned short debug, Base* botptr) : Protocol(server, conf, workDir, dumpLogs, debug, botptr) {}

unsigned int Client::apiVersion() {
	return 3000;
}

void Client::connectServer() {
	
}

void Client::disconnectServer(std::string reason) {
	
}

bool Client::isConnected() {
	
}

bool Client::deadServer() {
	
}

bool Client::isClient() {
	return true;
}

void Client::sendPrivMsg(const std::string& client, const std::string& target, const std::string& message) {
	
}

void Client::sendNotice(const std::string& client, const std::string& target, const std::string& message) {
	
}

void Client::sendCTCP(const std::string& client, const std::string& target, const std::string& ctcp, const std::string& params) {
	
}

void Client::sendCTCPReply(const std::string& client, const std::string& target, const std::string& ctcp, const std::string& params) {
	
}

void Client::setMode(const std::string& client, const std::string& target, const std::list<std::string>& setModes, const std::list<std::string>& remModes) {
	
}

void Client::setSNOmask(const std::string& client, bool add, char snomask) {
	
}

void Client::joinChan(const std::string& client, const std::string& channel, const std::string& key) {
	
}

void Client::partChan( const std::string& client, const std::string& channel, const std::string& reason) {
	
}

void Client::kickUser(const std::string& client, const std::string& channel, const std::string& user, const std::string& reason) {
	
}

std::string Client::addClient(std::string& nick, std::string& ident, std::string& host, std::string& gecos) {
	
}

void Client::removeClient(const std::string& client) {
	
}

void Client::setTopic(const std::string& client, const std::string& channel, const std::string& topic) {
	
}

void Client::inviteUser(const std::string& client, const std::string& channel, const std::string& user) {
	
}

void Client::knockOnChannel(const std::string& client, const std::string& channel, const std::string& reason) {
	
}

void Client::changeNick(const std::string& user, const std::string& newNick) {
	
}

void Client::sendPing(const std::string& remoteServer) {
	
}

void Client::operUp(const std::string& client, const std::string& usernameOrType, const std::string& password) {
	
}

void Client::sendServerNotice(char snomask, const std::string& message) {
	
}

void Client::setMetadata(const std::string& target, const std::string& key, const std::string& value) {
	
}

void Client::setXLine(const std::string& client, const std::string& lineType, const std::string& mask, time_t duration, const std::string& reason) {
	
}

void Client::remXLine(const std::string& client, const std::string& lineType, const std::string& mask) {
	
}

void Client::changeIdent(const std::string& user, const std::string& newIdent) {
	
}

void Client::changeHost(const std::string& user, const std::string& newHost) {
	
}

void Client::changeGecos(const std::string& user, const std::string& newGecos) {
	
}

void Client::sendWallops(const std::string& client, const std::string& message) {
	
}

void Client::sendOtherData(const std::string& client, const std::string& line) {
	
}

std::list<std::string> Client::networkServerList() {
	
}

std::list<std::string> Client::xLineTypes() {
	
}

std::list<std::string> Client::xLineList(const std::string& lineType) {
	
}

time_t Client::xLineExpiry(const std::string& lineType, const std::string& mask) {
	
}

std::string Client::xLineReason(const std::string& lineType, const std::string& mask) {
	
}

std::list<std::string> Client::chanListModes() {
	
}

std::list<std::string> Client::chanParamModes() {
	
}

std::list<std::string> Client::chanNoParamModes() {
	
}

std::list<std::pair<std::string, char>> Client::chanPrefixes() {
	
}

std::pair<std::string, char> Client::compareStatus(const std::string& status0, const std::string& status1) {
	
}

std::pair<std::string, char> Client::compareStatus(const std::string& status0, char status1) {
	
}

std::pair<std::string, char> Client::compareStatus(char status0, char status1) {
	
}

std::string Client::chanTopic(const std::string& channel) {
	
}

time_t Client::chanTimestamp(const std::string& channel) {
	
}

std::set<std::string> Client::chanUsers(const std::string& channel) {
	
}

bool Client::userInChan(const std::string& channel, const std::string& user) {
	
}

std::pair<std::string, char> Client::userStatus(const std::string& channel, const std::string& user) {
	
}

bool Client::userHasStatus(const std::string& channel, const std::string& user, const std::string& status) {
	
}

bool Client::userHasStatus(const std::string& channel, const std::string& user, char status) {
	
}

bool Client::userHasStatusOrGreater(const std::string& channel, const std::string& user, const std::string& status) {
	
}

bool Client::userHasStatusOrGreater(const std::string& channel, const std::string& user, char status) {
	
}

std::list<std::string> Client::chanModes(const std::string& channel) {
	
}

std::list<std::string> Client::chanListModeList(const std::string& channel, const std::string& listMode) {
	
}

bool Client::chanHasMode(const std::string& channel, const std::string& mode) {
	
}

std::string Client::chanModeParam(const std::string& channel, const std::string& mode) {
	
}

std::list<std::string> Client::clientList() {
	
}

std::string Client::clientNick(const std::string& client) {
	
}

std::string Client::userIdent(const std::string& user) {
	
}

std::string Client::userHost(const std::string& user) {
	
}

std::string Client::userGecos(const std::string& user) {
	
}

std::set<std::string> Client::userModes(const std::string& user) {
	
}

bool Client::userHasMode(const std::string& user, const std::string& mode) {
	
}

std::set<char> Client::userSNOmasks(const std::string& user) {
	
}

bool Client::userHasSNOmask(const std::string& user, char snomask) {
	
}

std::set<std::string> Client::userChans(const std::string& user) {
	
}

time_t Client::userTimestamp(const std::string& user) {
	
}

time_t Client::userNickTimestamp(const std::string& user) {
	
}

void Client::processedOutChanMsg(const std::string& client, const std::string& channel, char status, const std::string& message) {
	
}

void Client::processedOutUserMsg(const std::string& client, const std::string& nick, const std::string& message) {
	
}

void Client::processedOutChanNotice(const std::string& client, const std::string& channel, char status, const std::string& message) {
	
}

void Client::processedOutUserNotice(const std::string& client, const std::string& nick, const std::string& message) {
	
}

void Client::processedOutChanCTCP(const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params) {
	
}

void Client::processedOutUserCTCP(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) {
	
}

void Client::processedOutChanCTCPReply(const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params) {
	
}

void Client::processedOutUserCTCPReply(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) {
	
}