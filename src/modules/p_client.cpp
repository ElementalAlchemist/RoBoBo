#include "protocol.h"
#include <deque>

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
		std::unordered_map<std::string, char> statuses;
		time_t timestamp;
};

class LocalClient : public User {
	public:
		LocalClient(std::string clientid, std::string theNick, std::string theIdent, std::string theHost, std::string theGecos, Client* modClass);
		std::string id;
		std::string gecos;
		std::set<std::string> modes;
		std::set<char> snomasks;
		std::shared_ptr<Socket> connection;
		std::deque<std::string> sendQueue;
		std::thread receiveThread, sendThread, secondsThread;
		void receive();
		void send();
		void decrementSeconds();
		void sendLine(const std::string& line);
		std::atomic<unsigned int> seconds;
	private:
		Client* module;
};

User::User(std::string theNick, std::string theIdent, std::string theHost) : nick(theNick), ident(theIdent), host(theHost) {}

LocalClient::LocalClient(std::string clientid, std::string theNick, std::string theIdent, std::string theHost, std::string theGecos, Client* modClass) : User(theNick, theIdent, theHost), id(clientid), gecos(theGecos), module(modClass) {}

void LocalClient::receive() {
	
}

void LocalClient::send() {
	
}

void LocalClient::decrementSeconds() {
	while (true) {
		if (!connection->isConnected())
			return;
		std::this_thread::sleep_for(std::chrono::seconds(1));
		if (seconds > 0)
			seconds--;
	}
}

void LocalClient::sendLine(const std::string& line) {
	
}

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
		void setXLine(const std::string& client, const std::string& lineType, const std::string& mask, time_t duration, const std::string& reason);
		void remXLine(const std::string& client, const std::string& lineType, const std::string& mask);
		void sendWallops(const std::string& client, const std::string& message);
		void sendOtherData(const std::string& client, const std::string& line);
		
		std::list<std::string> xLineTypes();
		std::list<std::string> chanListModes();
		std::list<std::string> chanParamModes();
		std::list<std::string> chanNoParamModes();
		std::list<std::pair<std::string, char>> chanPrefixes();
		std::pair<std::string, char> compareStatus(const std::string& status0, const std::string& status1);
		std::pair<std::string, char> compareStatus(const std::string& status0, char status1);
		std::pair<std::string, char> compareStatus(char status0, char status1);
		
		std::list<std::string> chanList();
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
		std::set<std::string> userModes(const std::string& user);
		bool userHasMode(const std::string& user, const std::string& mode);
		std::set<char> userSNOmasks(const std::string& user);
		bool userHasSNOmask(const std::string& user, char snomask);
		std::set<std::string> userChans(const std::string& user);
		
		void processedOutChanMsg(const std::string& client, const std::string& channel, char status, const std::string& message);
		void processedOutUserMsg(const std::string& client, const std::string& nick, const std::string& message);
		void processedOutChanNotice(const std::string& client, const std::string& channel, char status, const std::string& message);
		void processedOutUserNotice(const std::string& client, const std::string& nick, const std::string& message);
		void processedOutChanCTCP(const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params);
		void processedOutUserCTCP(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params);
		void processedOutChanCTCPReply(const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params);
		void processedOutUserCTCPReply(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params);
	private:
		std::unordered_map<std::string, std::shared_ptr<User>> users;
		std::unordered_map<std::string, std::shared_ptr<Channel>> channels;
		std::unordered_map<std::string, std::shared_ptr<LocalClient>> connClients;
		bool floodControl;
		
		std::set<std::string> listModes;
		std::set<std::string> paramParamModes;
		std::set<std::string> paramModes;
		std::set<std::string> normalModes;
		std::list<std::pair<std::string, char>> prefixes;
		
		std::unordered_map<std::string, char> convertMode;
		std::unordered_map<char, std::string> convertChanMode, convertUserMode;
		
		void processIncoming(const std::string& client, const std::string& line);
		
		friend class LocalClient;
};

Client::Protocol(std::string server, std::map<std::string, std::string> conf, std::string workDir, bool dumpLogs, unsigned short debug, Base* botptr) : Protocol(server, conf, workDir, dumpLogs, debug, botptr) {}

unsigned int Client::apiVersion() {
	return 3000;
}

void Client::connectServer() {
	char floodSwitch = config["floodcontrol"][0];
	if (floodSwitch == 'n' || floodSwitch == 'N' || floodSwitch == '0' || floodSwitch == 'f' || floodSwitch == 'F')
		floodControl = false;
	else
		floodControl = true;
	
}

void Client::disconnectServer(std::string reason) {
	
}

bool Client::isConnected() {
	for (std::pair<std::string, std::shared_ptr<LocalClient>> client : connClients) {
		if (!client.second->connection->isConnected())
			return false;
	}
	return true;
}

bool Client::deadServer() {
	return connClients.empty();
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
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	std::list<char> setModesChar, remModesChar;
	std::string params = "";
	for (std::string mode : setModes) {
		size_t paramPos = mode.find('=');
		std::string param ("");
		if (paramPos != std::string::npos) {
			param = mode.substr(paramPos + 1);
			mode = mode.substr(0, paramPos);
		}
		std::unordered_map<std::string, char>::iterator convIter = convertMode.find(mode);
		if (convIter != convertMode.end()) {
			setModesChar.push_back(convIter);
			params += " " + param;
		}
	}
	for (std::string mode : remModes) {
		size_t paramPos = mode.find('=');
		std::string param ("");
		if (paramPos != std::string::npos) {
			param = mode.substr(paramPos + 1);
			mode = mode.substr(0, paramPos);
		}
		std::unordered_map<std::string, char>::iterator convIter = convertMode.find(mode);
		if (convIter != convertMode.end()) {
			remModesChar.push_back(convIter);
			params += " " + param;
		}
	}
	std::string modesStr = "";
	if (!setModesChar.empty()) {
		modesStr += "+";
		for (char modeChar : setModesChar)
			modesStr += modeChar;
	}
	if (!remModesChar.empty()) {
		modesStr += "-";
		for (char modeChar : remModesChar)
			modesStr += modeChar;
	}
	if (modesStr.empty())
		return;
	clientIter->second->sendLine("MODE " + target + " " + modesStr + params);
}

void Client::setSNOmask(const std::string& client, bool add, char snomask) {
	std::unordered_map<std::string, char>::iterator convIter = convertMode.find("snomask");
	if (convIter == convertMode.end())
		return;
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("MODE " + clientIter->second->nick + " +" + std::string(convIter->second) + " " + (add ? "+" : "-") + std::string(snomask));
}

void Client::joinChan(const std::string& client, const std::string& channel, const std::string& key) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	std::string joinLine = "JOIN " + channel;
	if (!key.empty())
		joinLine += " " + key;
	clientIter->second->sendLine(joinLine);
}

void Client::partChan( const std::string& client, const std::string& channel, const std::string& reason) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("PART " + channel + " :" + reason);
}

void Client::kickUser(const std::string& client, const std::string& channel, const std::string& user, const std::string& reason) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("KICK " + channel + " " + user + " :" + reason);
}

std::string Client::addClient(std::string& nick, std::string& ident, std::string& host, std::string& gecos) {
	// TODO: this
}

void Client::removeClient(const std::string& client) {
	// TODO: this
}

void Client::setTopic(const std::string& client, const std::string& channel, const std::string& topic) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("TOPIC " + channel + " :" + topic);
}

void Client::inviteUser(const std::string& client, const std::string& channel, const std::string& user) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("INVITE " + user + " " + channel);
}

void Client::knockOnChannel(const std::string& client, const std::string& channel, const std::string& reason) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("KNOCK " + channel + " :" + reason);
}

void Client::changeNick(const std::string& user, const std::string& newNick) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(user);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("NICK " + newNick);
}

void Client::sendPing(const std::string& remoteServer) {
	if (connClients.empty())
		return;
	connClients.begin()->second->sendLine("PING :" + remoteServer);
}

void Client::operUp(const std::string& client, const std::string& usernameOrType, const std::string& password) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("OPER " + usernameOrType + " " + password);
}

void Client::setXLine(const std::string& client, const std::string& lineType, const std::string& mask, time_t duration, const std::string& reason) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	std::ostringstream lineToSend;
	if (lineType.size() > 2)
		lineToSend << lineType;
	else
		lineToSend << lineType << "LINE";
	lineToSend << " " << mask << " " << duration << " :" << reason;
	clientIter->second->sendLine(lineToSend.str());
}

void Client::remXLine(const std::string& client, const std::string& lineType, const std::string& mask) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	if (lineType.size() > 2)
		clientIter->second->sendLine(lineType + " " + mask);
	else
		clientIter->second->sendLine(lineType + "LINE " + mask);
}

void Client::sendWallops(const std::string& client, const std::string& message) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("WALLOPS :" + message);
}

void Client::sendOtherData(const std::string& client, const std::string& line) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine(line);
}

std::list<std::string> Client::xLineTypes() {
	return std::list<std::string> { "G", "K", "Z", "E", "SHUN" };
}

std::list<std::string> Client::chanListModes() {
	std::list<std::string> modes;
	for (std::string mode : listModes)
		modes.push_back(mode);
	return modes;
}

std::list<std::string> Client::chanParamModes() {
	std::list<std::string> modes;
	for (std::string mode : paramModes)
		modes.push_back(mode);
	for (std::string mode : paramParamModes)
		modes.push_back(mode);
	return modes;
}

std::list<std::string> Client::chanNoParamModes() {
	std::list<std::string> modes;
	for (std::string mode : normalModes)
		modes.push_back(mode);
	return modes;
}

std::list<std::pair<std::string, char>> Client::chanPrefixes() {
	return prefixes;
}

std::pair<std::string, char> Client::compareStatus(const std::string& status0, const std::string& status1) {
	for (std::pair<std::string, char> status : prefixes) {
		if (status0 == status.first || status1 == status.first)
			return status;
	}
	return std::pair<std::string, char> ("", ' ');
}

std::pair<std::string, char> Client::compareStatus(const std::string& status0, char status1) {
	for (std::pair<std::string, char> status : prefixes) {
		if (status0 == status.first || status1 == status.second)
			return status;
	}
	return std::pair<std::string, char> ("", ' ');
}

std::pair<std::string, char> Client::compareStatus(char status0, char status1) {
	for (std::pair<std::string, char> status : prefixes) {
		if (status0 == status.second || status1 == status.second)
			return status;
	}
	return std::pair<std::string, char> ("", ' ');
}

std::list<std::string> chanList() {
	std::list<std::string> inChans;
	for (std::pair<std::string, std::shared_ptr<Channel>> channel : channels)
		inChans.push_back(channel.first);
	return inChans;
}

std::string Client::chanTopic(const std::string& channel) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return "";
	return chanIter->second->topic;
}

time_t Client::chanTimestamp(const std::string& channel) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return 0;
	return chanIter->second->timestamp;
}

std::set<std::string> Client::chanUsers(const std::string& channel) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return std::set<std::string> ();
	return chanIter->second->users;
}

bool Client::userInChan(const std::string& channel, const std::string& user) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return false;
	return (chanIter->second->users.find(user) != chanIter->second->users.end());
}

std::pair<std::string, char> Client::userStatus(const std::string& channel, const std::string& user) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return std::pair<std::string, char> ("", ' ');
	std::unordered_map<std::string, char>::iterator statusIter = chanIter->second->statuses.find(user);
	if (statusIter == chanIter->second->statuses.end())
		return std::pair<std::string, char> ("", ' ');
	for (std::pair<std::string, char> status : prefixes) {
		if (status.second == statusIter->second)
			return status;
	}
	// It should never be the case that we get down here, but...
	//  1. it avoids compiler warnings, and
	//  2. if some weird bug does happen, we do still have to return something.
	return std::pair<std::string, char> ("", ' ');
}

bool Client::userHasStatus(const std::string& channel, const std::string& user, const std::string& status) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return false;
	std::unordered_map<std::string, char>::iterator statusIter = chanIter->second->statuses.find(user);
	if (statusIter == chanIter->second->statuses.end())
		return false;
	for (std::pair<std::string, char> prefix : prefixes) {
		if (prefix.first == status)
			return (statusIter->second == prefix.second);
	}
	return false;
}

bool Client::userHasStatus(const std::string& channel, const std::string& user, char status) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return false;
	std::unordered_map<std::string, char>::iterator statusIter = chanIter->second->statuses.find(user);
	if (statusIter == chanIter->second->statuses.end())
		return false;
	return (statusIter->second == status);
}

bool Client::userHasStatusOrGreater(const std::string& channel, const std::string& user, const std::string& status) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return false;
	std::unordered_map<std::string, char>::iterator statusIter = chanIter->second->statuses.find(user);
	if (statusIter == chanIter->second->statuses.end())
		return false;
	for (std::pair<std::string, char> prefix : prefixes) {
		if (statusIter->second == prefix.second)
			return true;
		if (prefix.first == status)
			return false;
	}
	return false;
}

bool Client::userHasStatusOrGreater(const std::string& channel, const std::string& user, char status) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return false;
	std::unordered_map<std::string, char>::iterator statusIter = chanIter->second->statuses.find(user);
	if (statusIter == chanIter->second->statuses.end())
		return false;
	for (std::pair<std::string, char> prefix : prefixes) {
		if (statusIter->second == prefix.second)
			return true;
		if (prefix.second == status)
			return false;
	}
	return false;
}

std::list<std::string> Client::chanModes(const std::string& channel) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return std::list<std::string> ();
	return chanIter->second->modes;
}

std::list<std::string> Client::chanListModeList(const std::string& channel, const std::string& listMode) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return std::list<std::string> ();
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator listIter = chanIter->second->listModes.find(listMode);
	if (listIter == chanIter->second->listModes.end())
		return std::list<std::string> ();
	return listIter->second;
}

bool Client::chanHasMode(const std::string& channel, const std::string& mode) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return;
	for (std::string chanMode : chanIter->second->modes) {
		if (chanMode == mode)
			return true;
	}
	return false;
}

std::string Client::chanModeParam(const std::string& channel, const std::string& mode) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return;
	for (std::string chanMode : chanIter->second->modes) {
		size_t equals = chanMode.find('=');
		if (equals == std::string::npos)
			continue;
		if (chanMode.substr(0, equals) == mode)
			return chanMode.substr(equals + 1);
	}
	return "";
}

std::list<std::string> Client::clientList() {
	std::list<std::string> clients;
	for (std::pair<std::string, std::shared_ptr<LocalClient>> client : connClients)
		clients.push_back(client.first);
	return clients;
}

std::string Client::clientNick(const std::string& client) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return "";
	return clientIter->second->nick;
}

std::string Client::userIdent(const std::string& user) {
	std::unordered_map<std::string, std::shared_ptr<User>>::iterator userIter = users.find(user);
	if (userIter == users.end())
		return "";
	// TODO: also accept client ID as user
	return userIter->second->ident;
}

std::string Client::userHost(const std::string& user) {
	std::unordered_map<std::string, std::shared_ptr<User>>::iterator userIter = users.find(user);
	if (userIter == users.end())
		return "";
	// TODO: also accept client ID as user
	return userIter->second->host;
}

std::set<std::string> Client::userModes(const std::string& user) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(user);
	if (clientIter == connClients.end())
		return std::set<std::string> ();
	return clientIter->second->modes;
}

bool Client::userHasMode(const std::string& user, const std::string& mode) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(user);
	if (clientIter == connClients.end())
		return false;
	return (clientIter->second->modes.find(mode) != clientIter->second->modes.end());
}

std::set<char> Client::userSNOmasks(const std::string& user) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(user);
	if (clientIter == connClients.end())
		return std::set<char>();
	return clientIter->second->snomasks;
}

bool Client::userHasSNOmask(const std::string& user, char snomask) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(user);
	if (clientIter == connClients.end())
		return false;
	return (clientIter->second->snomasks.find(snomask) != clientIter->second->snomasks.end());
}

std::set<std::string> Client::userChans(const std::string& user) {
	std::unordered_map<std::string, std::shared_ptr<User>>::iterator userIter = users.find(user);
	if (userIter == users.end())
		return std::set<std::string> ();
	// TODO: also accept client ID as user
	return userIter->second->channels;
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

void Client::processIncoming(const std::string& client, const std::string& line) {
	
}