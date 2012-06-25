#include "protocol.h"
#include <algorithm>
#include <atomic>
#include <deque>

class Client;
class User {
	public:
		std::string nick;
		std::string ident;
		std::string host;
		std::string gecos;
		std::set<std::string> channels;
};

class Channel {
	public:
		std::string name;
		std::string topic;
		std::set<std::string> modes;
		std::unordered_map<std::string, std::list<std::string>> listModes;
		std::set<std::string> users;
		std::unordered_map<std::string, std::list<std::string>> prefixes;
};

class LocalClient : public User {
	public:
		LocalClient(Client* clientptr);
		std::string identifier;
		std::string altNick;
		std::set<std::string> modes;
		std::set<char> snomasks;
		std::string socketType;
		std::string password;
		std::string server;
		Socket* connection;
		void receiveData();
		void sendData();
		void decreaseSeconds();
		std::atomic<unsigned int> seconds;
		std::deque<std::string> messageQueue;
		std::thread receiveThread, sendThread, secondsThread;
	private:
		Client* protoptr;
};

LocalClient::LocalClient(Client* clientptr) : protoptr(clientptr) {}

void LocalClient::receiveData() {
	connection->sendData("NICK " + nick);
	connection->sendData("USER " + ident + " localhost " + server + " :" + gecos);
	if (password != "")
		connection->sendData("PASS " + password);
	while (true) {
		
	}
}

void LocalClient::sendData() {
	while (true) {
		
	}
}

void LocalClient::decreaseSeconds() {
	while (true) {
		if (connection == NULL || !connection->isConnected())
			break;
		std::this_thread::sleep_for(std::chrono::seconds(1));
		if (seconds > 0)
			seconds--;
	}
}

class Client : public Protocol {
	public:
		Client(std::string server, std::map<std::string, std::string> conf, std::string workDir, bool dumpLogs, unsigned short debug, Base* botptr);
		~Client();
		unsigned int apiVersion();
		void connectServer();
		void disconnectServer(std::string reason);
		bool isConnected();
		bool deadServer();
		bool isClient();
		void sendPrivMsg(std::string client, std::string target, std::string message);
		void sendNotice(std::string client, std::string target, std::string message);
		void sendCTCP(std::string client, std::string target, std::string ctcp, std::string params);
		void sendCTCPReply(std::string client, std::string target, std::string ctcp, std::string params);
		void setMode(std::string client, std::string target, std::list<std::string> setModes, std::list<std::string> delModes);
		void setSNOMask(std::string client, char snomask, bool add);
		void setChanTopic(std::string client, std::string channel, std::string topic);
		void joinChannel(std::string client, std::string channel, std::string key = "");
		void partChannel(std::string client, std::string channel, std::string reason);
		void kickUser(std::string client, std::string channel, std::string nick, std::string reason);
		void changeNick(std::string client, std::string newNick);
		std::string addClient(std::string nick, std::string ident, std::string host, std::string gecos);
		void removeClient(std::string client);
		void oper(std::string client, std::string username, std::string password);
		void setXLine(std::string client, std::string linetype, std::string mask, time_t duration, std::string reason);
		void delXLine(std::string client, std::string linetype, std::string mask);
		void sendOtherData(std::string client, std::string line);
		
		void processedChanMsg(std::string client, std::string target, char status, std::string message);
		void processedUserMsg(std::string client, std::string target, std::string message);
		void processedChanNotice(std::string client, std::string target, char status, std::string message);
		void processedUserNotice(std::string client, std::string target, std::string message);
		void processedChanCTCP(std::string client, std::string target, char status, std::string ctcp);
		void processedUserCTCP(std::string client, std::string target, std::string ctcp);
		void processedChanCTCPReply(std::string client, std::string target, char status, std::string ctcp);
		void processedUserCTCPReply(std::string client, std::string target, std::string ctcp);
		
		std::list<std::string> listModes();
		std::list<std::string> paramModes();
		std::list<std::string> modes();
		std::list<std::pair<std::string, char>> statuses();
		std::set<char> channelTypes();
		std::list<std::string> channels();
		std::list<std::string> inChannels(std::string client);
		std::list<std::string> channelUsers(std::string channel);
		bool userInChannel(std::string channel, std::string user);
		std::string channelTopic(std::string channel);
		std::list<std::string> channelModes(std::string channel);
		bool channelHasMode(std::string channel, std::string mode);
		std::string modeParam(std::string channel, std::string mode);
		std::list<std::string> channelListMode(std::string channel, std::string mode);
		bool channelListHasEntry(std::string channel, std::string listMode, std::string entry);
		std::pair<std::string, char> userStatus(std::string channel, std::string user);
		std::pair<std::string, char> compareStatus(std::string status0, std::string status1);
		std::pair<std::string, char> compareStatus(std::string status0, char status1);
		std::pair<std::string, char> compareStatus(char status0, std::string status1);
		std::pair<std::string, char> compareStatus(char status0, char status1);
		bool userHasStatus(std::string channel, std::string user, std::string status);
		bool userHasStatus(std::string channel, std::string user, char status);
		bool userHasStatusOrGreater(std::string channel, std::string user, std::string status);
		bool userHasStatusOrGreater(std::string channel, std::string user, char status);
		std::list<std::string> clients();
		std::list<std::string> userModes(std::string client);
		bool hasUserMode(std::string client, std::string mode);
		std::list<char> snomasks(std::string client);
		bool hasSNOMask(std::string client, char snomask);
		std::list<std::string> userChannels(std::string nick);
		
		void processIncoming(std::string client, std::string line);
		bool floodThrottle;
	private:
		std::unordered_map<std::string, User*> users;
		std::unordered_map<std::string, Channel*> channels;
		std::unordered_map<std::string, LocalClient*> clients;
		std::mutex dataProcess;
		
		std::set<char> chanTypes;
		std::set<std::string> chanListModes;
		std::set<std::string> chanParamParamModes;
		std::set<std::string> chanParamModes;
		set::set<std::string> chanOtherModes;
		std::list<std::pair<std::string, char>> chanPrefixes;
		std::map<std::string, char> modeConvertLong;
		std::map<char, std::string> modeConvertChan, modeConvertUser;
		size_t maxModes;
		
		void saveMode(std::string longName, char shortChar, bool chan);
};

Client::Client(std::string server, std::map<std::string, std::string> conf, std::string workDir, bool dumpLogs, unsigned short debug, Base* botptr) : Protocol(server, conf, workDir, dumpLogs, debug, botptr) {}

Client::~Client() {
	
}

unsigned int Client::apiVersion() {
	return 3000;
}

void Client::connectServer() {
	unsigned int i = 0;
	std::ostringstream entryNum;
	entryNum << i;
	std::map<std::string, std::string>::iterator confIter = config.find(entryNum.str() + "/id");
	while (confIter != config.end()) {
		std::map<std::string, std::string>::iterator confNickIter = config.find(entryNum.str() + "/nick"), confIdentIter = config.find(entryNum.str() + "/ident"), confGecosIter = config.find(entryNum.str() + "/gecos"), confPortIter = config.find(entryNum.str() + "/port"), confSockIter = config.find(entryNum.str() + "/sockettype");
		if (confNickIter == config.end() || confIdentIter == config.end() || confGecosIter == config.end() || confPortIter == config.end() || confSockIter == config.end()) {
			i++;
			entryNum.str("");
			entryNum << i;
			confIter = config.find(entryNum.str() + "/id");
			continue;
		}
		LocalClient* newClient = new LocalClient(this);
		newClient->identifier = confIter->second;
		newClient->nick = confNickIter->second;
		newClient->ident = confIdentIter->second;
		newClient->gecos = confGecosIter->second;
		newClient->seconds = 0;
		newClient->socketType = config[entryNum.str() + "/sockettype"];
		newClient->connection = assignSocket(newClient->socketType);
		if (newClient->connection == NULL) {
			delete newClient;
			i++;
			entryNum.str("");
			entryNum << i;
			confIter = config.find(entryNum.str() + "/id");
			continue;
		}
		newClient->connection->connectServer(serverName, confPortIter->second);
		if (!newClient->connection->isConnected()) {
			killSocket(newClient->socketType, newClient->connection);
			delete newClient;
			i++;
			entryNum.str("");
			entryNum << i;
			confIter = config.find(entryNum.str() + "/id");
			continue;
		}
		newClient->receiveThread = std::thread(&LocalClient::receiveData, newClient);
		std::string floodValue = config["floodthrottle"];
		std::transform(floodValue.begin(), floodValue.end(), floodValue.begin(), ::tolower);
		if (floodValue == "no" || floodValue == "false" || floodValue == "n") {
			floodThrottle = false;
			newClient->sendThread = std::thread(&LocalClient::sendData, newClient);
			newClient->secondsThread = std::thread(&LocalClient::decreaseSeconds, newClient);
		} else
			floodThrottle = true;
		newClient->altNick = config[entryNum.str() + "/altnick"]; // Fill in the alt nick and password with an empty string if they don't exist
		newClient->password = config[entryNum.str() + "/password"]; // Unlike the other fields, they are not required.
		newClient->server = serverName;
		clients.insert(std::pair<std::string, LocalClient*> (newClient->identifier, newClient));
		i++;
		entryNum.str("");
		entryNum << i;
		confIter = config.find(entryNum.str() + "/id");
	}
}

void Client::disconnectServer(std::string reason) {
	for (std::pair<std::string, LocalClient*> client : clients) {
		client.second->connection->sendData("QUIT :" + reason);
		client.second->connection->closeConnection();
		if (client.second->receiveThread.joinable())
			client.second->receiveThread.join();
		if (client.second->sendThread.joinable())
			client.second->sendThread.join();
		if (client.second->secondsThread.joinable())
			client.second->secondsThread.join();
		killSocket(client.second->socketType, client.second->connection);
		delete client.second;
	}
}

bool Client::isConnected() {
	// If at least one client is still connected, we're technically still connected to the server.
	for (std::pair<std::string, LocalClient*> client : clients) {
		if (client.second->connection->isConnected())
			return true;
	}
	return false;
}

bool Client::deadServer() {
	return clients.empty();
}

bool Client::isClient() {
	return true;
}

void Client::sendPrivMsg(std::string client, std::string target, std::string message) {
	if (clients.find(client) == clients.end())
		return;
	if (chanTypes.find(target[0]) == chanTypes.end()) {
		if (chanTypes.size() > 1) { // Make sure before we check the second char that what we're checking is of a length > 1, i.e. not a single-char nick or something
			for (std::pair<std::string, char> prefix : chanPrefixes) {
				if (target[0] == prefix.second) {
					if (chanTypes.find(target[1]) == chanTypes.end()) {
						callChanMsgOutHook(client, target.substr(1), target[0], message);
						return;
					}
					break;
				}
			}
		}
		callUserMsgOutHook(client, target, message);
	} else
		callChanMsgOutHook(client, target, ' ', message);
}

void Client::sendNotice(std::string client, std::string target, std::string message) {
	if (clients.find(client) == clients.end())
		return;
	if (chanTypes.find(target[0]) == chanTypes.end()) {
		if (chanTypes.size() > 1) {
			for (std::pair<std::string, char> prefix : chanPrefixes) {
				if (target[0] == prefix.second) {
					if (chanTypes.find(target[1]) == chanTypes.end()) {
						callChanNoticeOutHook(client, target.substr(1), target[0], message);
						return;
					}
					break;
				}
			}
		}
		callUserNoticeOutHook(client, target, message);
	} else
		callChanNoticeOutHook(client, target, ' ', message);
}

void Client::sendCTCP(std::string client, std::string target, std::string ctcp, std::string params) {
	if (clients.find(client) == clients.end())
		return;
	if (chanTypes.find(target[0]) == chanTypes.end()) {
		if (chanTypes.size() > 1) {
			for (std::pair<std::string, char> prefix : chanPrefixes) {
				if (target[0] == prefix.second) {
					if (chanTypes.find(target[1]) == chanTypes.end()) {
						callChanCTCPOutHook(client, target.substr(1), target[0], ctcp, params);
						return;
					}
					break;
				}
			}
		}
		callUserCTCPOutHook(client, target, ctcp, params);
	} else
		callChanCTCPOutHook(client, target, ' ', ctcp, params);
}

void Client::sendCTCPReply(std::string client, std::string target, std::string ctcp, std::string params) {
	if (clients.find(client) == clients.end())
		return;
	if (chanTypes.find(target[0]) == chanTypes.end()) {
		if (chanTypes.size() > 1) {
			for (std::pair<std::string, char> prefix : chanPrefixes) {
				if (target[0] == prefix.second) {
					if (chanTypes.find(target[1]) == chanTypes.end()) {
						callChanCTCPReplyOutHook(client, target.substr(1), target[0], ctcp, params);
						return;
					}
					break;
				}
			}
		}
		callUserCTCPOutHook(client, target, ctcp, params);
	} else
		callChanCTCPOutHook(client, target, ' ', ctcp, params);
}

void Client::setMode(std::string client, std::string target, std::list<std::string> setModes, std::list<std::string> delModes) {
	
}

void Client::setSNOMask(std::string client, char snomask, bool add) {
	
}

void Client::setChanTopic(std::string client, std::string channel, std::string topic) {
	
}

void Client::joinChannel(std::string client, std::string channel, std::string key = "") {
	
}

void Client::partChannel(std::string client, std::string channel, std::string reason) {
	
}

void Client::kickUser(std::string client, std::string channel, std::string nick, std::string reason) {
	
}

void Client::changeNick(std::string client, std::string newNick) {
	
}

std::string Client::addClient(std::string nick, std::string ident, std::string host, std::string gecos) {
	
}

void Client::removeClient(std::string client) {
	
}

void Client::oper(std::string client, std::string username, std::string password) {
	
}

void Client::setXLine(std::string client, std::string linetype, std::string mask, time_t duration, std::string reason) {
	
}

void Client::delXLine(std::string client, std::string linetype, std::string mask) {
	
}

void Client::sendOtherData(std::string client, std::string line) {
	
}

void Client::processedChanMsg(std::string client, std::string target, char status, std::string message) {
	
}

void Client::processedUserMsg(std::string client, std::string target, std::string message) {
	
}

void Client::processedChanNotice(std::string client, std::string target, char status, std::string message) {
	
}

void Client::processedUserNotice(std::string client, std::string target, std::string message) {
	
}

void Client::processedChanCTCP(std::string client, std::string target, char status, std::string ctcp) {
	
}

void Client::processedUserCTCP(std::string client, std::string target, std::string ctcp) {
	
}

void Client::processedChanCTCPReply(std::string client, std::string target, char status, std::string ctcp) {
	
}

void Client::processedUserCTCPReply(std::string client, std::string target, std::string ctcp) {
	
}

std::list<std::string> Client::listModes() {
	
}

std::list<std::string> Client::paramModes() {
	
}

std::list<std::string> Client::modes() {
	
}

std::list<std::pair<std::string, char>> Client::statuses() {
	
}

std::set<char> Client::channelTypes() {
	
}

std::list<std::string> Client::channels() {
	
}

std::list<std::string> Client::inChannels(std::string client) {
	
}

std::list<std::string> Client::channelUsers(std::string channel) {
	
}

bool Client::userInChannel(std::string channel, std::string user) {
	
}

std::string Client::channelTopic(std::string channel) {
	
}

std::list<std::string> Client::channelModes(std::string channel) {
	
}

bool Client::channelHasMode(std::string channel, std::string mode) {
	
}

std::string Client::modeParam(std::string channel, std::string mode) {
	
}

std::list<std::string> Client::channelListMode(std::string channel, std::string mode) {
	
}

bool Client::channelListHasEntry(std::string channel, std::string listMode, std::string entry) {
	
}

std::pair<std::string, char> Client::userStatus(std::string channel, std::string user) {
	
}

std::pair<std::string, char> Client::compareStatus(std::string status0, std::string status1) {
	
}

std::pair<std::string, char> Client::compareStatus(std::string status0, char status1) {
	
}

std::pair<std::string, char> Client::compareStatus(char status0, std::string status1) {
	
}

std::pair<std::string, char> Client::compareStatus(char status0, char status1) {
	
}

bool Client::userHasStatus(std::string channel, std::string user, std::string status) {
	
}

bool Client::userHasStatus(std::string channel, std::string user, char status) {
	
}

bool Client::userHasStatusOrGreater(std::string channel, std::string user, std::string status) {
	
}

bool Client::userHasStatusOrGreater(std::string channel, std::string user, char status) {
	
}

std::list<std::string> Client::clients() {
	
}

std::list<std::string> Client::userModes(std::string client) {
	
}

bool Client::hasUserMode(std::string client, std::string mode) {
	
}

std::list<char> Client::snomasks(std::string client) {
	
}

bool Client::hasSNOMask(std::string client, char snomask) {
	
}

std::list<std::string> Client::userChannels(std::string nick) {
	
}

void Client::processIncoming(std::string client, std::string line) {
	dataProcess.lock();
	
	dataProcess.unlock();
}

void Client::saveMode(std::string longName, char shortChar, bool chan) {
	std::map<std::string, std::string>::iterator confIter = config.find("mode-" + longName);
	if (confIter != config.end()) {
		if (confIter->second == "disabled")
			return;
		if (confIter->second != "")
			shortChar = confIter->second[0];
	}
	modeConvertLong[longName] = shortChar;
	if (chan)
		modeConvertChan[shortChar] = longName;
	else
		modeConvertUser[shortChar] = longName;
}

PROTOCOL_SPAWN(Client)