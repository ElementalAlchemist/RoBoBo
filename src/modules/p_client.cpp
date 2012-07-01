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
	// Send the initial data here because the send queue may not always be called, depending on whether flood throttle is active.
	connection->sendData("CAP LS");
	if (password != "")
		connection->sendData("PASS " + password);
	connection->sendData("NICK " + nick);
	connection->sendData("USER " + ident + " localhost " + server + " :" + gecos);
	while (connection != NULL && connection->isConnected()) {
		std::string message = connection->receive();
		if (message == "")
			break;
		protoptr->processIncoming(identifier, message);
	}
}

void LocalClient::sendData() {
	while (connection != NULL && connection->isConnected()) {
		if (messageQueue.empty()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			continue;
		}
		std::string outgoingMsg = messageQueue.front();
		messageQueue.pop_front();
		std::vector<std::string> splitMsg = protoptr->parseIRC(outgoingMsg);
		unsigned int secondsToAdd = 1;
		protoptr->callSendHooks(identifier, splitMsg);
		std::string command = splitMsg[0];
		// These values were borrowed and mapped as well as possible to a simple mapping of command -> penalty
		// Some commands have values that vary depending on output or other things (e.g. WHO by number of lines of output)
		// so they were mapped to the most common value or best approximation.
		if (command == "GLINE" || command == "KLINE" || command == "OJOIN" || command == "PING" || command == "PONG" || command == "QLINE" || command == "SAJOIN" || command == "SAKICK" || command == "SAMODE" || command == "SANICK" || command == "SAPART" || command == "SAQUIT" || command == "SATOPIC" || command == "ZLINE")
			secondsToAdd = 0;
		else if (command == "JOIN" || command == "MAP" || command == "REHASH" || command == "TOPIC" || command == "WHO" || command == "WHOIS" || command == "WHOWAS")
			secondsToAdd = 2;
		else if (command == "CYCLE")
			secondsToAdd = 3;
		else if (command == "INVITE" || command == "NICK")
			secondsToAdd = 4;
		else if (command == "KNOCK" || command == "LIST" || command == "PART")
			secondsToAdd = 5;
		if (seconds + secondsToAdd > 10)
			std::this_thread::sleep_for(std::chrono::seconds(seconds + secondsToAdd - 10));
		seconds += secondsToAdd;
		// TODO: Split the line if it's too long
		connection->sendData(outgoingMsg);
	}
}

void LocalClient::decreaseSeconds() {
	while (connection != NULL && connection->isConnected()) {
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
		void processedChanCTCP(std::string client, std::string target, char status, std::string ctcp, std::string params);
		void processedUserCTCP(std::string client, std::string target, std::string ctcp, std::string params);
		void processedChanCTCPReply(std::string client, std::string target, char status, std::string ctcp, std::string params);
		void processedUserCTCPReply(std::string client, std::string target, std::string ctcp, std::string params);
		
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
		std::vector<std::string> parseIRC(std::string line);
		void callSendHooks(std::string client, std::vector<std::string> parsedLine);
		bool floodThrottle;
	private:
		std::unordered_map<std::string, User*> users;
		std::unordered_map<std::string, Channel*> channels;
		std::unordered_map<std::string, LocalClient*> clients;
		std::mutex dataProcess;
		std::string defaultSocket;
		std::string defaultPort;
		std::string defaultBind;
		
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
		std::string getNextID();
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
	defaultSocket = config["sockettype"];
	defaultPort = config["port"];
	defaultBind = config["bind"];
	while (confIter != config.end()) {
		std::map<std::string, std::string>::iterator confNickIter = config.find(entryNum.str() + "/nick"), confIdentIter = config.find(entryNum.str() + "/ident"), confGecosIter = config.find(entryNum.str() + "/gecos"), confPortIter = config.find(entryNum.str() + "/port"), confSockIter = config.find(entryNum.str() + "/sockettype");
		if (confNickIter == config.end() || confIdentIter == config.end() || confGecosIter == config.end() || (confPortIter == config.end() && defaultPort.empty()) || (confSockIter == config.end() && defaultSocket.empty()) {
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
		newClient->socketType = confSockIter->second;
		if (newClient->socketType == "")
			newClient->socketType = defaultSocket;
		std::string bindAddr = config[entryNum.str() + "/bind"];
		if (bindAddr == "")
			bindAddr = defaultBind; // This may also be an empty string if we are not to bind to anything in particular.
		newClient->connection = assignSocket(newClient->socketType);
		if (newClient->connection == NULL) {
			delete newClient;
			i++;
			entryNum.str("");
			entryNum << i;
			confIter = config.find(entryNum.str() + "/id");
			continue;
		}
		std::string port = confPortIter->second;
		if (port.empty())
			port = defaultPort;
		newClient->connection->connectServer(serverName, port, bindAddr);
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
	std::unordered_map<std::string, LocalClient*>::iterator clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	bool channel = true;
	if (chanTypes.find(target[0]) == chanTypes.end())
		channel = false;
	std::string modeString = "", params = "";
	if (!setModes.empty()) {
		modeString += '+';
		for (std::string mode : setModes) {
			size_t equals = mode.find_first_of('=');
			if (equals == std::string::npos) {
				if (chanOtherModes.find(mode) == chanOtherModes.end())
					continue;
				modeString += modeConvertLong(mode);
			} else {
				std::string modeName = mode.substr(0, equals), param = mode.substr(equals + 1);
				if (chanOtherModes.find(modeName) != chanOtherModes.end()) {
					modeString += modeConvertLong(modeName);
					continue;
				}
				if (chanListModes.find(modeName) == chanListModes.end() && chanParamModes.find(modeName) == chanParamModes.end() && chanParamParamModes.find(modeName) == chanParamParamModes.end()) {
					bool statusChar = false;
					for (std::pair<std::string, char> status : chanPrefixes) {
						if (status.first == modeName)
							statusChar = true;
					}
					if (statusChar) {
						modeString += modeConvertLong(modeName);
						params += " " + param;
					}
					continue;
				}
				modeString += modeConvertLong(modeName);
				params += " " + param;
			}
		}
	}
	if (!delModes.empty()) {
		modeString += '-';
		for (std::string mode : delModes) {
			size_t equals = mode.find_first_of('=');
			if (equals == std::string::npos) {
				if (chanOtherModes.find(mode) == chanOtherModes.end() && chanParamModes.find(mode) == chanParamModes.end())
					continue;
				modeString += modeConvertLong(mode);
			} else {
				std::string modeName = mode.substr(0, equals), param = mode.substr(equals + 1);
				if (chanOtherModes.find(modeName) != chanOtherModes.end() || chanParamModes.find(modeName) != chanParamModes.end()) {
					modeString += modeConvertLong(modeName);
					continue;
				}
				if (chanListModes.find(modeName) == chanListModes.end() && chanParamParamModes.find(modeName) == chanParamParamModes.end()) {
					bool statusChar = false;
					for (std::pair<std::string, char> status : chanPrefixes) {
						if (status.first == modeName)
							statusChar = true;
					}
					if (statusChar) {
						modeString += modeConvertLong(modeName);
						params += " " + param;
					}
					continue;
				}
				modeString += modeConvertLong(modeName);
				params += " " + param;
			}
		}
	}
	if (modeString != "") { // The mode string could be empty if we were passed entirely invalid modes or no modes
		if (floodThrottle)
			clientIter->second->messageQueue.push_back("MODE " + target + " " + modeString + params);
		else
			clientIter->second->connection->sendData("MODE " + target + " " + modeString + params);
	}
}

void Client::setSNOMask(std::string client, char snomask, bool add) {
	std::unordered_map<std::string, LocalClient*>::iterator clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	char snomode = modeConvertLong("snomask");
	if (snomode == ' ')
		return; // this server does not have snomasks
	std::ostringstream modeStr;
	modeStr << "MODE " << clientIter->second->nick << " +" << snomode << " " << (add ? '+' : '-') << snomask;
	if (floodThrottle)
		clientIter->second->messageQueue.push_back(modeStr.str());
	else
		clientIter->second->connection->sendData(modeStr.str());
}

void Client::setChanTopic(std::string client, std::string channel, std::string topic) {
	std::unordered_map<std::string, LocalClient*>::iterator clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	if (floodThrottle)
		clientIter->second->messageQueue.push_back("TOPIC " + channel + " :" + topic);
	else
		clientIter->second->connection->sendData("TOPIC " + channel + " :" + topic);
}

void Client::joinChannel(std::string client, std::string channel, std::string key = "") {
	std::unordered_map<std::string, LocalClient*>::iterator clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	if (floodThrottle)
		clientIter->second->messageQueue.push_back("JOIN " + channel + (key ? " " + key : ""));
	else
		clientIter->second->connection->sendData("JOIN " + channel + (key ? " " + key : ""));
}

void Client::partChannel(std::string client, std::string channel, std::string reason) {
	std::unordered_map<std::string, LocalClient*>::iterator clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	if (floodThrottle)
		clientIter->second->messageQueue.push_back("PART " + channel + " :" + reason);
	else
		clientIter->second->connection->sendData("PART " + channel + " :" + reason);
}

void Client::kickUser(std::string client, std::string channel, std::string nick, std::string reason) {
	std::unordered_map<std::string, LocalClient*>::iterator clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	if (floodThrottle)
		clientIter->second->messageQueue.push_back("KICK " + channel + " " + nick + " :" + reason);
	else
		clientIter->second->connection->sendData("KICK " + channel + " " + nick + " :" + reason);
}

void Client::changeNick(std::string client, std::string newNick) {
	std::unordered_map<std::string, LocalClient*>::iterator clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	if (floodThrottle)
		clientIter->second->messageQueue.push_back("NICK " + newNick);
	else
		clientIter->second->connection->sendData("NICK " + newNick);
}

std::string Client::addClient(std::string nick, std::string ident, std::string host, std::string gecos) {
	Socket* newConnection = assignSocket(defaultSocket);
	if (newConnection == NULL)
		return "";
	LocalClient* newClient = new LocalClient (this);
	newClient->nick = nick;
	newClient->ident = ident;
	newClient->gecos = gecos;
	newClient->connection = newConnection;
	newClient->server = serverName;
	newClient->seconds = 0;
	newConnection->connectServer(serverName, defaultPort, defaultBind);
	if (!newConnection->isConnected()) {
		delete newConnection;
		delete newClient;
		return "";
	}
	std::string id = getNextID();
	clients.insert(std::pair<std::string, LocalClient*> (id, newClient));
	return id;
}

void Client::removeClient(std::string client) {
	std::unordered_map<std::string, LocalClient*>::iterator clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	clientIter->second->connection->closeConnection();
	// Wait for the threads to end
	if (clientIter->second->receiveThread.joinable())
		clientIter->second->receiveThread.join();
	if (clientIter->second->sendThread.joinable())
		clientIter->second->sendThread.join();
	if (clientIter->second->secondsThread.joinable())
		clientIter->second->secondsThread.join();
	delete clientIter->second->connection;
	delete clientIter->second;
}

void Client::oper(std::string client, std::string username, std::string password) {
	std::unordered_map<std::string, LocalClient*>::iterator clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	if (floodThrottle)
		clientIter->second->messageQueue.push_back("OPER " + username + " " + password);
	else
		clientIter->second->connection->sendData("OPER " + username + " " + password);
}

// TODO: Figure out how to have users set up the x:line format in configure
void Client::setXLine(std::string client, std::string linetype, std::string mask, time_t duration, std::string reason) {
	std::unordered_map<std::string, LocalClient*>::iterator clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	std::ostringstream lineStr;
	if (linetype.size() == 1) {
		lineStr << linetype << "LINE " << mask << " " << duration << " :" << reason;
		if (floodThrottle)
			clientIter->second->messageQueue.push_back(lineStr.str());
		else
			clientIter->second->connection->sendData(lineStr.str());
	} else {
		lineStr << linetype << " " << mask << " " << duration << " :" << reason;
		if (floodThrottle)
			clientIter->second->messageQueue.push_back(lineStr.str());
		else
			clientIter->second->connection->sendData(lineStr.str());
	}
}

void Client::delXLine(std::string client, std::string linetype, std::string mask) {
	std::unordered_map<std::string, LocalClient*>::iterator clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	if (linetype.size() == 1) {
		if (floodThrottle)
			clientIter->second->messageQueue.push_back(linetype + "LINE " + mask);
		else
			clientIter->second->connection->sendData(linetype + "LINE " + mask);
	} else {
		if (floodThrottle)
			clientIter->second->messageQueue.push_back(linetype + " " + mask);
		else
			clientIter->second->connection->sendData(linetype + " " + mask);
	}
}

void Client::sendOtherData(std::string client, std::string line) {
	std::unordered_map<std::string, LocalClient*>::iterator clientIter = clients.find(client);
	if (clientIter == clients.end())
		return;
	if (floodThrottle)
		clientIter->second->messageQueue.push_back(line);
	else
		clientIter->second->connection->sendData(line);
}

void Client::processedChanMsg(std::string client, std::string target, char status, std::string message) {
	std::ostringstream outStr;
	outStr << "PRIVMSG " << (status == ' ' ? "" : status) << target << " :" << message;
	if (floodThrottle)
		clients.find(client)->second->messageQueue.push_back(outStr.str());
	else {
		clients.find(client)->second->connection->sendData(outStr.str());
		callChanMsgSendHook(client, target, status, message);
	}
}

void Client::processedUserMsg(std::string client, std::string target, std::string message) {
	if (floodThrottle)
		clients.find(client)->second->messageQueue.push_back("PRIVMSG " + target + " :" + message);
	else {
		clients.find(client)->second->connection->sendData("PRIVMSG " + target + " :" + message);
		callUserMsgSendHook(client, target, message);
	}
}

void Client::processedChanNotice(std::string client, std::string target, char status, std::string message) {
	std::ostringstream outStr;
	outStr << "NOTICE " << (status == ' ' ? "" : status) << target << " :" << message;
	if (floodThrottle)
		clients.find(client)->second->messageQueue.push_back(outStr.str());
	else {
		clients.find(client)->second->connection->sendData(outStr.str());
		callChanNoticeSendHook(client, target, status, message);
	}
}

void Client::processedUserNotice(std::string client, std::string target, std::string message) {
	if (floodThrottle)
		clients.find(client)->second->messageQueue.push_back("NOTICE " + target + " :" + message);
	else {
		clients.find(client)->second->connection->sendData("NOTICE " + target + " :" + message);
		callUserNoticeSendHook(client, target, message);
	}
}

void Client::processedChanCTCP(std::string client, std::string target, char status, std::string ctcp, std::string params) {
	std::ostringstream outStr;
	if (status == ' ') {
		if (params == "")
			outStr << "PRIVMSG " << target << " :" << (char)1 << ctcp << (char)1;
		else
			outStr << "PRIVMSG " << target << " :" << (char)1 << ctcp << " " << params << (char)1;
	} else {
		if (params == "")
			outStr << "PRIVMSG " << status << target << " :" << (char)1 << ctcp << (char)1;
		else
			outStr << "PRIVMSG " << status << target << " :" << (char)1 << ctcp << " " << params << (char)1;
	}
	if (floodThrottle)
		clients.find(client)->second->messageQueue.push_back(outStr.str());
	else {
		clients.find(client)->second->connection->sendData(outStr.str());
		callChanCTCPSendHook(client, target, status, ctcp, params);
	}
}

void Client::processedUserCTCP(std::string client, std::string target, std::string ctcp, std::string params) {
	if (floodThrottle)
		clients.find(client)->second->messageQueue.push_back("PRIVMSG " + target + " :\x01" + ctcp + (params == "" ? "" : " " + params) + "\x01");
	else {
		clients.find(client)->second->connection->sendData("PRIVMSG " + target + " :\x01" + ctcp + (params == "" ? "" : " " + params) + "\x01");
		callUserCTCPSendHook(client, target, ctcp, params);
	}
}

void Client::processedChanCTCPReply(std::string client, std::string target, char status, std::string ctcp, std::string params) {
	std::ostringstream outStr;
	if (status == ' ') {
		if (params == "")
			outStr << "NOTICE " << target << " :" << (char)1 << ctcp << (char)1;
		else
			outStr << "NOTICE " << target << " :" << (char)1 << ctcp << " " << params << (char)1;
	} else {
		if (params == "")
			outStr << "NOTICE " << status << target << " :" << (char)1 << ctcp << (char)1;
		else
			outStr << "NOTICE " << status << target << " :" << (char)1 << ctcp << " " << params << (char)1;
	}
	if (floodThrottle)
		clients.find(client)->second->messageQueue.push_back(outStr.str());
	else {
		clients.find(client)->second->connection->sendData(outStr.str());
		callChanCTCPReplySendHook(client, target, status, ctcp, params);
	}
}

void Client::processedUserCTCPReply(std::string client, std::string target, std::string ctcp, std::string params) {
	if (floodThrottle)
		clients.find(client)->second->messageQueue.push_back("NOTICE " + target + " :\x01" + ctcp + (params == "" ? "" : " " + params) + "\x01");
	else {
		clients.find(client)->second->connection->sendData("NOTICE " + target + " :\x01" + ctcp + (params == "" ? "" : " " + params) + "\x01");
		callUserCTCPReplySendHook(client, target, ctcp, params);
	}
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

std::vector<std::string> Client::parseIRC(std::string line) {
	std::vector<std::string> parsedLine;
	std::string currentToken = "";
	for (std::string::iterator lineIter = line.begin(); lineIter != line.end(); ++lineIter) {
		if (currentToken == "" && *lineIter == ':') {
			for (; lineIter != line.end(); ++lineIter)
				currentToken += *lineIter;
			parsedLine.push_back(currentToken);
			currentToken = "";
		} else if (*lineIter == ' ') {
			parsedLine.push_back(currentToken);
			currentToken = "";
		} else
			currentToken += *lineIter;
	}
	if (currentToken != "")
		parsedLine.push_back(currentToken);
	return parsedLine;
}

void Client::callSendHooks(std::string client, std::vector<std::string> parsedLine) {
	if (parsedLine[0] == "PRIVMSG") {
		if (parsedLine[2][0] == (char)1) {
			std::string ctcp = parsedLine[2], params = "";
			if (ctcp[ctcp.size() - 1] == (char)1)
				ctcp = ctcp.substr(1, ctcp.size() - 2);
			else
				ctcp = ctcp.substr(1);
			size_t spacePos = ctcp.find_first_of(' ');
			if (spacePos != std::string::npos) {
				params = ctcp.substr(spacePos + 1);
				ctcp = ctcp.substr(0, spacePos);
			}
			if (chanTypes.find(parsedLine[1][0]) == chanTypes.end()) {
				bool isChannel = false;
				if (chanTypes.find(parsedLine[1][1]) != chanTypes.end()) {
					for (std::pair<std::string, char> prefix : chanPrefixes) {
						if (prefix.second == parsedLine[1][0]) {
							isChannel = true;
							callChanCTCPSendHook(client, parsedLine[1].substr(1), parsedLine[1][0], ctcp, params);
							break;
						}
					}
				}
				if (!isChannel)
					callUserCTCPSendHook(client, parsedLine[1], ctcp, params);
			} else
				callChanCTCPSendHook(client, parsedLine[1], ' ', ctcp, params);
		} else {
			if (chanTypes.find(parsedLine[1][0]) == chanTypes.end()) {
				bool isChannel = false;
				if (chanTypes.find(parsedLine[1][1]) != chanTypes.end()) {
					for (std::pair<std::string, char> prefix : chanPrefixes) {
						if (prefix.second == parsedLine[1][0]) {
							isChannel = true;
							callChanMsgSendHook(client, parsedLine[1].substr(1), parsedLine[1][0], parsedLine[2]);
							break;
						}
					}
				}
				if (!isChannel)
					callUserMsgSendHook(client, parsedLine[1], parsedLine[2]);
			} else
				callUserMsgSendHook(client, parsedLine[1], ' ', parsedLine[2]);
		}
	} else if (parsedLine[0] == "NOTICE") {
		if (parsedLine[2][0] == (char)1) {
			std::string ctcp = parsedLine[2], params = "";
			if (ctcp[ctcp.size() - 1] == (char)1)
				ctcp = ctcp.substr(1, ctcp.size() - 2);
			else
				ctcp = ctcp.substr(1);
			size_t spacePos = ctcp.find_first_of(' ');
			if (spacePos != std::string::npos) {
				params = ctcp.substr(spacePos + 1);
				ctcp = ctcp.substr(0, spacePos);
			}
			if (chanTypes.find(parsedLine[1][0]) == chanTypes.end()) {
				bool isChannel = false;
				if (chanTypes.find(parsedLine[1][1] != chanTypes.end())) {
					for (std::pair<std::string, char> prefix : chanPrefixes) {
						if (prefix.second == parsedLine[1][0]) {
							isChannel = true;
							callChanCTCPReplySendHook(client, parsedLine[1].substr(1), parsedLine[1][0], ctcp, params);
							break;
						}
					}
				}
				if (!isChannel)
					callUserCTCPReplySendHook(client, parsedLine[1], ctcp, params);
			} else
				callChanCTCPReplySendHook(client, parsedLine[1], ' ', ctcp, params);
		} else {
			if (chanTypes.find(parsedLine[1][0]) == chanTypes.end()) {
				bool isChannel = false;
				if (chanTypes.find(parsedLine[1][1] != chanTypes.end())) {
					for (std::pair<std::string, char> prefix : chanPrefixes) {
						if (prefix.second == parsedLine[1][0]) {
							isChannel = true;
							callChanNoticeSendHook(client, parsedLine[1].substr(1), parsedLine[1][0], parsedLine[2]);
							break;
						}
					}
				}
				if (!isChannel)
					callUserNoticeSendHook(client, parsedLine[1], parsedLine[2]);
			} else
				callChanNoticeSendHook(client, parsedLine[1], ' ', parsedLine[2]);
		}
	}
}

void Client::saveMode(std::string longName, char shortChar, bool chan) {
	std::map<std::string, std::string>::iterator confIter = config.find("mode-" + longName);
	if (confIter != config.end()) {
		if (confIter->second == "disabled")
			return; // Don't add the mode if it's disabled by the user.
		if (confIter->second != "")
			shortChar = confIter->second[0];
	}
	modeConvertLong[longName] = shortChar;
	if (chan)
		modeConvertChan[shortChar] = longName;
	else
		modeConvertUser[shortChar] = longName;
}

std::string Client::getNextID() {
	
}

PROTOCOL_SPAWN(Client)