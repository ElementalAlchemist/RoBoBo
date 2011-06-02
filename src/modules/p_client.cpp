#include "protoinclude.h"
#include <algorithm>

class Client;
class User {
	public:
		User(std::string ident, std::string host, Client* serverClass);
		void ident(std::string ident);
		std::string ident();
		void host(std::string host);
		std::string host();
		void addChannel(std::string channel);
		void removeChannel(std::string channel);
		std::list<std::string> channelList();
		void status(std::string channel, char status, bool adding);
		std::string status(std::string channel);
	private:
		std::string userIdent, userHost;
		std::tr1::unordered_map<std::string, std::set<char> > channels;
		Client* server;
};
class Client : public Protocol {
	friend class User;
	public:
		Client(std::string serverAddress, std::tr1::unordered_map<std::string, std::string> confVars, Base* theBase, unsigned short debug);
		~Client();
		unsigned int apiVersion();
		void connectServer();
		bool isClient();
		std::list<std::pair<std::string, char> > prefixes();
		std::set<char> channelTypes();
		std::vector<std::vector<std::string> > channelModes();
		std::list<std::string> channels();
		std::string channelTopic(std::string channel);
		std::set<std::string> channelUsers(std::string channel);
		std::string userIdent(std::string user);
		std::string userHost(std::string user);
		std::pair<std::string, char> userStatus(std::string channel, std::string user);
		char compareStatus(std::set<char> statuses);
		void sendMsg(std::string client, std::string target, std::string message);
		void sendNotice(std::string client, std::string target, std::string message);
		void setMode(std::string client, std::string target, std::string mode);
		void removeMode(std::string client, std::string target, std::string mode);
		void joinChannel(std::string client, std::string channel, std::string key = "");
		void partChannel(std::string client, std::string channel, std::string reason = "");
		void quitServer(std::string reason);
		void kickUser(std::string client, std::string channel, std::string user, std::string reason);
		void changeNick(std::string client, std::string newNick);
		void sendOther(std::string client, std::string rawLine);
		std::list<std::string> clients();
		std::tr1::unordered_map<std::string, std::string> clientInfo(std::string client);
		std::list<std::string> userModes(std::string client);
	private:
		pthread_t receiveThread, sendThread, secondsThread;
		pthread_attr_t detachedState;
		pthread_mutex_t secondsmutex;
		static void* handleData_thread(void* ptr);
		void handleData();
		static void* sendData_thread(void* ptr);
		void sendData();
		static void* secondsDecrement_thread(void* ptr);
		void secondsDecrement();
		std::queue<std::string> dataToSend;
		volatile unsigned int seconds;
		bool registered, altChanged, quitHooked, floodControl;
		std::tr1::unordered_map<std::string, User*> users;
		std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > > inChannels;
		std::tr1::unordered_map<std::string, bool> readingNames;
		std::list<std::string> uModes;
		std::list<std::pair<char, char> > statusPrefixes;
		std::vector<std::vector<char> > chanModes;
		std::set<char> chanTypes;
		unsigned int maxModes;
		void setChanMode(bool addMode, bool list, std::string channel, std::string mode, std::string param = "");
		void setStatus(bool addMode, std::string channel, std::string status, std::string user);
		char convertMode(std::string mode);
		std::string convertChanMode(char mode);
		std::string convertUserMode(char mode);
		void parse005(std::vector<std::string> parsedLine);
		void parseNames(std::string channel, std::string namesList);
};

User::User(std::string ident, std::string host, Client* serverClass) : userIdent(ident), userHost(host), server(serverClass) {}

void User::ident(std::string ident) {
	if (ident != "")
		userIdent = ident;
}

std::string User::ident() {
	return userIdent;
}

void User::host(std::string host) {
	if (host != "")
		userHost = host;
}

std::string User::host() {
	return userHost;
}

void User::addChannel(std::string channel) {
	channels.insert(std::pair<std::string, std::set<char> > (channel, std::set<char> ());
}

void User::removeChannel(std::string channel) {
	channels.erase(channels.find(channel));
}

std::list<std::string> User::channelList() {
	std::list<std::string> listOfChannels;
	for (std::tr1::unordered_map<std::string, char>::iterator chanIter = channels.begin(); chanIter != channels.end(); ++chanIter)
		listOfChannels.push_back(chanIter->first);
	return listOfChannels;
}

void User::status(std::string channel, char status, bool adding) {
	if (channels.find(channel) == channels.end())
		return;
	if (adding)
		channels.find(channel)->second.insert(status);
	else
		channels.find(channel)->second.erase(channels.find(channel)->second.find(status));
}

char User::status(std::string channel) {
	if (channels.find(channel) == channels.end())
		return '0';
	std::set<char> statusChars = channels.find(channel)->second;
	std::set<std::string> statuses;
	for (std::set<char>::iterator statIter = statusChars.begin(); statIter != statusChars.end(); ++statIter)
		statuses.insert(server->convertChanMode(*statIter));
	return server->compareStatus(statuses);
}

Client::Client(std::string serverAddress, std::tr1::unordered_map<std::string, std::string> confVars, Base* theBase, unsigned short debug) : Protocol(serverAddress, confVars, theBase, debug), maxModes(1) {
	pthread_mutex_init(&secondsmutex, NULL); // initialize mutex for use in sending threads
	pthread_attr_init(&detachedState);
	pthread_attr_setdetachstate(&detachedState, PTHREAD_CREATE_DETACHED);
	if (serverConf["bind"] != "") {
		if (!connection->bindSocket(serverConf["bind"]))
			std::cout << "Could not bind to " << serverConf["bind"] << "; trying without binding.  Abort RoBoBo and adjust configuration settings to try again with binding." << std::endl; // debug level 1
	}
	if (confVars["floodcontrol"][0] == 'n' || confVars["floodcontrol"][0] == 'N')
		floodControl = false;
	else
		floodControl = true;
}

Client::~Client() {
	pthread_cancel(receiveThread);
	pthread_cancel(sendThread);
	pthread_cancel(secondsThread);
}

unsigned int Client::apiVersion() {
	return 2000;
}

void Client::connectServer() {
	if (connection == NULL) {
		std::cout << "p_client: " << serverName << ": Socket handle could not be obtained." << std::endl;
		keepServer = false;
		return;
	}
	botBase->callPreConnectHook(serverName);
	std::istringstream portNumber (serverConf["port"]);
	unsigned short port;
	portNumber >> port;
	connection.connectServer(serverName, port);
	sleep(1); // don't send right away in case of some sort of death or slowness
	botBase->callConnectHook(serverName);
	if (serverConf["password"] != "")
		sendOther("PASS " + serverConf["password"]);
	changeNick(serverConf["nick"]);
	sendOther("USER " + serverConf["ident"] + " here " + serverName + " :" + serverConf["gecos"]);
	pthread_create(&receiveThread, &detachedState, handleData_thread, this);
	pthread_create(&sendThread, &detachedState, sendData_thread, this);
}

bool Client::isClient() {
	return true;
}

std::list<std::pair<std::string, char> > Client::prefixes() {
	return statusPrefixes;
}

std::set<char> Client::channelTypes() {
	return chanTypes;
}

std::vector<std::vector<std::string> > Client::channelModes() {
	std::vector<std::vector<std::string> > pubChanModes;
	for (size_t i = 0; i < chanModes.size(); i++) {
		std::vector<std::string> theseChanModes;
		for (size_t j = 0; j < chanModes[i].size(); j++)
			theseChanModes.push_back(convertChanMode(chanModes[i][j]));
		pubChanModes.push_back(theseChanModes);
	}
	return pubChanModes;
}

std::list<std::string> Client::channels() {
	std::list<std::string> retChannels;
	for (std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > >::iterator chanIter = inChannels.begin(); chanIter != inChannels.end(); ++chanIter)
		retChannels.push_back(chanIter->first);
	return retChannels;
}

std::string Client::channelTopic(std::string channel) {
	std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > >::iterator chanIter = inChannels.find(channel);
	if (chanIter == inChannels.end())
		return "";
	return chanIter->second->first;
}

std::set<std::string> Client::channelUsers(std::string channel) {
	std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > >::iterator chanIter = inChannels.find(channel);
	if (chanIter == inChannels.end())
		return std::set<std::string>();
	return chanIter->second.second.second;
}

std::string Client::userIdent(std::string user) {
	std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(user);
	if (userIter == users.end())
		return "";
	return userIter->second->ident();
}

std::string Client::userHost(std::string user) {
	std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(user);
	if (userIter == users.end())
		return "";
	return userIter->second->host();
}

std::pair<std::string, char> Client::userStatus(std::string channel, std::string user) {
	if (users.find(user) == users.end())
		return std::pair<std::string, char> ("", ' ');
	if (inChannels.find(channel) == inChannels.end())
		return std::pair<std::string, char> ("", ' ');
	std::string status = users.find(user)->second->status(channel);
	if (status == "")
		return std::pair<std::string, char> ("", ' ');
	for (std::list<std::pair<char, char> >::iterator statIter = statusPrefixes.begin(); statIter != statusPrefixes.end(); ++statIter) {
		if (*statIter.first == convertMode(status))
			return std::pair<std::string, char> (status, (*statIter).second);
	}
	return std::pair<std::string, char> ("", ' '); // Um, some bug perhaps?
}

std::string Client::compareStatus(std::set<std::string> statuses) {
	for (std::list<std::pair<char, char> >::iterator statIter = statusPrefixes.begin(); statIter != statusPrefixes.end(); ++statIter) {
		std::string thisStatus = convertChanMode(statIter->first);
		if (statuses.find(thisStatus))
			return thisStatus;
	}
	return "";
}

void Client::sendMsg(std::string client, std::string target, std::string message) {
	dataToSend.push("PRIVMSG " + target + " :" + message);
}

void Client::sendNotice(std::string client, std::string target, std::string message) {
	dataToSend.push("NOTICE " + target + " :" + message);
}

void Client::setMode(std::string client, std::string target, std::string mode) {
	dataToSend.push("MODE " + target + " +" + mode);
}

void Client::removeMode(std::string client, std::string target, std::string mode) {
	dataToSend.push("MODE " + target + " -" + mode);
}

void Client::joinChannel(std::string client, std::string channel, std::string key) {
	if (key == "")
		dataToSend.push("JOIN " + channel);
	else
		dataToSend.push("JOIN " + channel + " " + key);
}

void Client::partChannel(std::string client, std::string channel, std::string reason) {
	dataToSend.push("PART " + channel + " :" + reason);
}

void Client::quitServer(std::string reason) {
	dataToSend.push("QUIT :" + reason);
}

void Client::kickUser(std::string client, std::string channel, std::string user, std::string reason) {
	dataToSend.push("KICK " + channel + " " + user + " :" + reason);
}

void Client::changeNick(std::string client, std::string newNick) {
	dataToSend.push("NICK " + newNick);
}

void Client::sendOther(std::string client, std::string rawLine) {
	dataToSend.push(rawLine);
}

void* Client::handleData_thread(void* ptr) {
	Client* protomod = (Client*) ptr;
	protomod->handleData();
	return NULL;
}

void Client::handleData() {
	std::string receivedLine = "";
	std::vector<std::string> parsedLine;
	while (true) {
		receivedLine = connection.receive();
		if (receivedLine == "")
			break; // this case indicates a receive error
		if (debugLevel >= 3)
			std::cout << receivedLine << std::endl;
		parsedLine = botBase->parseLine(receivedLine);
		botBase->callPreHook(serverName, parsedLine); // call module hooks for the received message
		if (parsedLine[1] == "001") { // welcome to the network
			sendOther("MODE " + serverConf["nick"] + " +B"); // set bot mode
			if (serverConf["channels"] != "")
				joinChannel(serverConf["channels"]);
			registered = true;
			sendOther("WHOIS " + serverConf["nick"]);
		} else if (parsedLine[1] == "005") // server features
			parse005(parsedLine);
		else if (parsedLine[1] == "311" && parsedLine[3] == serverConf["nick"]) {
			serverConf["ident"] = parsedLine[4];
			serverConf["host"] = parsedLine[5];
		} else if (parsedLine[1] == "332") { // channel topic
			std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > >::iterator it = inChannels.find(parsedLine[3]);
			if (it != inChannels.end())
				it->second.first = parsedLine[4];
		} else if (parsedLine[1] == "352") { // WHO reply
			std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(parsedLine[7]);
			if (userIter != users.end()) {
				userIter->second->ident(parsedLine[4]);
				userIter->second->host(parsedLine[5]);
			}
		} else if (parsedLine[1] == "353") // NAMES reply
			parseNames(parsedLine[4], parsedLine[5]);
		else if (parsedLine[1] == "366") // end of NAMES reply
			readingNames[parsedLine[3]] = false;
		else if (parsedLine[1] == "433" && !registered) { // nickname already in use
			if (!altChanged) {
				changeNick(serverConf["altnick"]);
				serverConf["nick"] = serverConf["altnick"];
				altChanged = true;
			} else
				quitServer("");
		} else if (parsedLine[1] == "MODE") {
			bool addMode = true;
			if (parsedLine[2] == serverConf["nick"]) { // if it's a user mode
				for (unsigned int i = 0; i < parsedLine[3].size(); i++) {
					if (parsedLine[3][i] == '+')
						addMode = true;
					else if (parsedLine[3][i] == '-')
						addMode = false;
					else {
						if (addMode)
							uModes.push_back(convertUserMode(parsedLine[3][i]));
						else {
							for (std::list<std::string>::iterator uModeIter = uModes.begin(); uModeIter != uModes.end(); ++uModeIter) {
								if (convertUserMode(parsedLine[3][i]) == *uModeIter) {
									uModes.erase(uModeIter);
									break;
								}
							}
						}
					}
				}
			} else { // it's a channel mode
				int currParam = 4; // MODE parameters start at the fourth IRC parameter
				for (unsigned int i = 0; i < parsedLine[3].size(); i++) {
					if (parsedLine[3][i] == '+')
						addMode = true;
					else if (parsedLine[3][i] == '-')
						addMode = false;
					else {
						int category;
						bool found = false, prefix = false;
						for (unsigned int j = 0; j < statusPrefixes.size(); j++) {
							if (statusPrefixes[j].first == parsedLine[3][i]) {
								category = 0; // count it as a list mode since it's a list of users who hold a status
								found = true;
								prefix = true;
							}
						}
						if (!found) {
							for (unsigned int j = 0; j < chanModes.size(); j++) {
								for (unsigned int k = 0; k < chanModes[j].size(); k++) {
									if (parsedLine[3][i] == chanModes[j][k]) {
										found = true;
										category = j;
										break;
									}
								}
								if (found)
									break;
							}
							if (!found)
								category = 4;
						}
						
						if (category == 0) {
							std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > >::iterator chanIter = inChannels.find(parsedLine[2]);
							if (chanIter != inChannels.end()) {
								if (prefix)
									setStatus(addMode, chanIter->first, convertChanMode(parsedLine[3][i]), parsedLine[currParam++]);
								else
									setChanMode(addMode, true, chanIter->first, convertChanMode(parsedLine[3][i]), parsedLine[currParam]);
							}
						} else if (category == 1 || (category == 2 && addMode)) {
							std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > >::iterator chanIter = inChannels.find(parsedLine[2]);
							if (chanIter != inChannels.end())
								setChanMode(addMode, false, chanIter->first, convertChanMode(parsedLine[3][i]), parsedLine[currParam++]);
						} else {
							std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > >::iterator chanIter = inChannels.find(parsedLine[2]);
							if (chanIter != inChannels.end())
								setChanMode(addMode, false, chanIter->first, convertChanMode(parsedLine[3][i]));
						}
					}
				}
			}
		} else if (parsedLine[1] == "NICK" && serverConf["nick"] == parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1)) // bot's nick changed
			serverConf["nick"] = parsedLine[2];
		else if (parsedLine[1] == "NICK") {
			std::string oldNick = parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1);
			for (std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > >::iterator chanIter = inChannels.begin(); chanIter != inChannels.end(); ++chanIter) {
				if (chanIter->second.second.second.find(oldNick) != chanIter->second.second.second.end()) {
					chanIter->second.second.second.erase(oldNick);
					chanIter->second.second.second.insert(parsedLine[2]);
				}
			}
			User* nickChanger = users.find(oldNick)->second;
			users.erase(oldNick);
			users.insert(std::pair<std::string, User*> (parsedLine[2], nickChanger));
		} else if (parsedLine[1] == "JOIN" && serverConf["nick"] == parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1)) { // bot joined a channel
			inChannels.insert(std::pair<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > > (parsedLine[2], std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > ()));
			sendOther("WHO " + parsedLine[2]);
		} else if (parsedLine[1] == "JOIN") {
			std::string hostmask = parsedLine[0].substr(1);
			std::string nick = hostmask.substr(0, hostmask.find_first_of('!'));
			if (users.find(nick) == users.end()) {
				size_t exclaim = hostmask.find_first_of('!');
				std::string ident = hostmask.substr(exclaim, hostmask.find_first_of('@') - exclaim);
				std::string host = hostmask.substr(hostmask.find_first_of('@'));
				users.insert(std::pair<std::string, std::string> (nick, new User(ident, host)));
			}
			inChannels.find(parsedLine[2])->second.second.second.insert(nick);
		} else if (parsedLine[1] == "PART" && serverConf["nick"] == parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1))
			inChannels.erase(parsedLine[2]);
		else if (parsedLine[1] == "PART")
			inChannels.find(parsedLine[2])->second.second.second.erase(parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1));
		else if (parsedLine[1] == "QUIT" && serverConf["nick"] == parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1)) {
			connection.closeConnection();
			quitHooked = true;
			keepServer = false;
			break;
		} else if (parsedLine[1] == "QUIT") {
			for (std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > >::iterator chanIter = inChannels.begin(); chanIter != inChannels.end(); ++chanIter)
				chanIter->second.second.second.erase(parsedLine[0].substr(1, parsedLine[0].find_first_of('!')));
		} else if (parsedLine[1] == "KILL" && serverConf["nick"] == parsedLine[2]) {
			connection.closeConnection();
			quitHooked = true;
			keepServer = false;
			break;
		} else if (parsedLine[0] == "PING") // server ping
			sendOther("PONG " + parsedLine[1]);
		botBase->callPostHook(serverName, parsedLine);
	}
}

void* Client::sendData_thread(void* ptr) {
	Client* protomod = (Client*) ptr;
	protomod->sendData();
	return NULL;
}

void Client::sendData() {
	seconds = 0;
	unsigned short secondsToAdd = 0;
	std::string sendingMessage = "";
	std::vector<std::string> parsedLine;
	std::string command = "";
	pthread_create(&secondsThread, &detachedState, secondsDecrement_thread, this);
	while (true) {
		if (!connection.isConnected())
			break; // Thread must die when server isn't connected.
		if (dataToSend.empty()) {
			usleep(100000); // sleep for a short time to avoid processor abuse while being ready for data to arrive
			continue; // check again for empty queue
		}
		sendingMessage = dataToSend.front();
		dataToSend.pop();
		parsedLine = botBase->parseLine(sendingMessage);
		command = parsedLine[0];
		if (command == "PRIVMSG" || command == "NOTICE") {
			std::string newMessage = botBase->callHookOut(serverName, parsedLine);
			if (newMessage == "")
				continue; // do not send this canceled message
			sendingMessage = parsedLine[0] + " " + parsedLine[1] + " :" + newMessage;
			parsedLine = botBase->parseLine(sendingMessage);
		}
		if (command == "MODE") {
			secondsToAdd = 1;
			std::string tempStr = "";
			std::vector<std::string> splitLine;
			for (unsigned int i = 0; i < sendingMessage.size(); i++) {
				if (sendingMessage[i] == ' ') {
					splitLine.push_back(tempStr);
					tempStr = "";
					continue;
				}
				tempStr += sendingMessage[i];
			}
			if (tempStr != "")
				splitLine.push_back(tempStr);
			if (splitLine.size() > maxModes) {
				unsigned int i = 0;
				std::vector<std::string> keepModes;
				std::string newModeCommand = "MODE " + parsedLine[1];
				for (; i < maxModes; i++)
					keepModes.push_back(splitLine[i]);
				for (; i < splitLine.size(); i++)
					newModeCommand += " " + splitLine[i];
				splitLine = keepModes;
				dataToSend.push(newModeCommand);
			}
			std::string newCommand = "MODE " + parsedLine[1] + " ", params = "";
			bool adding = true;
			for (size_t i = 0; i < splitLine.size(); i++) {
				if (splitLine[i][0] == '+') {
					if (!adding || i == 0) {
						adding = true;
						newCommand += '+';
					}
				} else if (splitLine[i][0] == '-') {
					if (adding) {
						adding = false;
						newCommand += '-';
					}
				}
				char modeChar = convertMode(splitLine[i].substr(1));
				if (modeChar == ' ') // provided mode does not exist/does not match a mode letter; discard mode
					continue;
				newCommand += modeChar;
				if (splitLine[i].find_first_of('=') != std::string::npos)
					params += " " + splitLine[i].substr(splitLine[i].find_first_of('=') + 1);
			}
			sendingMessage = newCommand + params;
			for (unsigned int i = 0; i < splitLine.size(); i++) {
				if (splitLine[i] == "cloak")
					secondsToAdd = 6; // because cloaking/setting umode +x is apparently such an expensive operation.
			}
		} else { // MODE processes its own penalty addition
			if (command == "GLINE" || command == "KLINE" || (command == "NICK" && !registered) || command == "PASS" || command == "PING" || command == "PONG" || command == "QLINE" || command == "USER" || command == "ZLINE" || command == "OJOIN" || command == "SAJOIN" || command == "SAKICK" || command == "SAMODE" || command == "SANICK" || command == "SAPART" || command == "SAQUIT" || command == "SATOPIC")
				secondsToAdd = 0;
			else if (command == "JOIN" || command == "MAP" || command == "REHASH" || command == "TOPIC" || command == "WHO" || command == "WHOIS" || command == "WHOWAS")
				secondsToAdd = 2;
			else if (command == "CYCLE")
				secondsToAdd = 3;
			else if (command == "INVITE" || command == "NICK")
				secondsToAdd = 4;
			else if (command == "LIST" || command == "PART" || command == "KNOCK" || command == "MKPASSWD")
				secondsToAdd = 5;
			else // all commands not on the list are worth 1 second.
				secondsToAdd = 1;
		}
		std::string hostInfo = ":" + serverConf["nick"] + "!" + serverConf["ident"] + "@" + serverConf["host"] + " ";
		if (hostInfo.size() + sendingMessage.size() > 510) {
			std::string extraMessage = sendingMessage.substr(510 - hostInfo.size());
			sendingMessage = sendingMessage.substr(0, 510 - hostInfo.size()) + "\r\n";
			for (unsigned int i = 0; i < parsedLine.size() - 1; i++)
				sendingMessage += parsedLine[i] + " ";
			sendingMessage += extraMessage;
			secondsToAdd *= 2;
		}
		while (seconds + secondsToAdd > 10)
			sleep(1);
		connection.sendData(sendingMessage);
		if (debugLevel >= 3)
			std::cout << " -> " << sendingMessage << std::endl;
		botBase->callHookSend(serverName, parsedLine);
		if (command == "QUIT") {
			connection.closeConnection();
			keepServer = false;
			break;
		}
		pthread_mutex_lock(&secondsmutex);
		seconds += secondsToAdd;
		pthread_mutex_unlock(&secondsmutex);
	}
}

void* Client::secondsDecrement_thread(void* ptr) {
	Client* protomod = (Client*) ptr;
	protomod->secondsDecrement();
	return NULL;
}

void Client::secondsDecrement() {
	while (true) {
		if (!connection.isConnected()) {
			if (!quitHooked)
				botBase->callQuitHook(serverName);
			quitHooked = true;
			break; // thread must die when server isn't connected anymore
		}
		sleep(1);
		if (seconds > 0) {
			pthread_mutex_lock(&secondsmutex);
			seconds--;
			pthread_mutex_unlock(&secondsmutex);
		}
	}
}

void Client::setChanMode(bool addMode, bool list, std::string channel, std::string mode, std::string param) {
	std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > >::iterator chanIter = inChannels.find(channel);
	if (chanIter == inChannels.end())
		return;
	if (addMode) {
		if (list) {
			chanIter->second.second.first.insert(mode + "=" + param);
			return;
		}
		for (std::list<std::string>::iterator modeIter = chanIter->second.second.first.begin(); modeIter != chanIter->second.second.first.end(); ++modeIter) {
			std::string storedMode = *modeIter;
			if (storedMode.find_first_of('=') != std::string::npos)
				storedMode = storedMode.substr(0, storedMode.find_first_of('='));
			if (storedMode == mode) {
				chanIter->second.second.first.erase(modeIter);
				break;
			}
		}
		if (param == "")
			chanIter->second.second.first.insert(mode);
		else
			chanIter->second.second.first.insert(mode + "=" + param);
	} else {
		if (list || param != "") {
			std::list<std::string>::iterator foundMode = std::find(chanIter->second.second.first.begin(), chanIter->second.second.first.end(), mode + "=" + param);
			chanIter->second.second.first.erase(foundMode);
		} else {
			std::list<std::string>::iterator foundMode = std::find(chanIter->second.second.first.begin(), chanIter->second.second.first.end(), mode);
			chanIter->second.second.first.erase(foundMode);
		}
	}
}

void Client::setStatus(bool addMode, std::string channel, std::string status, std::string user) {
	std::tr1::unordered_map<std::string, User*>::iterator rankedUser = users.find(user);
	if (rankedUser = users.end())
		return;
	rankedUser->second->status(channel, status, addMode);
}

char Client::convertMode(std::string mode) {
	if (mode.find_first_of('=') != std::string::npos)
		mode = mode.substr(0, mode.find_first_of('='));
	if (mode == "admin")
		return 'a';
	if (mode == "allowinvite")
		return 'A';
	if (mode == "auditorium")
		return 'u';
	if (mode == "autoop")
		return 'w';
	if (mode == "ban")
		return 'b';
	if (mode == "banexception")
		return 'e';
	if (mode == "blockcaps")
		return 'B';
	if (mode == "blockcolor")
		return 'c';
	if (mode == "bot")
		return 'B';
	if (mode == "c_registered")
		return 'r';
	if (mode == "callerid")
		return 'g';
	if (mode == "censor")
		return 'G';
	if (mode == "chanacl")
		return 'W';
	if (mode == "cloak")
		return 'x';
	if (mode == "deaf")
		return 'd';
	if (mode == "deaf_commonchan")
		return 'c';
	if (mode == "delayjoin")
		return 'D';
	if (mode == "delaymsg")
		return 'd';
	if (mode == "exemtpchanops")
		return 'X';
	if (mode == "filter")
		return 'g';
	if (mode == "flood")
		return 'f';
	if (mode == "founder")
		return 'q';
	if (mode == "halfop")
		return 'h';
	if (mode == "hidechans")
		return 'I';
	if (mode == "hideoper")
		return 'H';
	if (mode == "history")
		return 'H';
	if (mode == "invex")
		return 'I';
	if (mode == "invisible")
		return 'i';
	if (mode == "inviteonly")
		return 'i';
	if (mode == "joinflood")
		return 'j';
	if (mode == "key")
		return 'k';
	if (mode == "kicknorejoin")
		return 'J';
	if (mode == "limit")
		return 'l';
	if (mode == "moderated")
		return 'm';
	if (mode == "nickflood")
		return 'F';
	if (mode == "noautoop")
		return 'n';
	if (mode == "noctcp")
		return 'C';
	if (mode == "noextmsg")
		return 'n';
	if (mode == "nokick")
		return 'Q';
	if (mode == "noknock")
		return 'K';
	if (mode == "nonick")
		return 'N';
	if (mode == "nonotice")
		return 'T';
	if (mode == "official-join")
		return 'Y';
	if (mode == "op")
		return 'o';
	if (mode == "oper")
		return 'o';
	if (mode == "operonly")
		return 'O';
	if (mode == "opmoderated")
		return 'U';
	if (mode == "permanent")
		return 'P';
	if (mode == "private")
		return 'p';
	if (mode == "redirect")
		return 'L';
	if (mode == "regdeaf")
		return 'R';
	if (mode == "reginvite")
		return 'R';
	if (mode == "regmoderated")
		return 'M';
	if (mode == "secret")
		return 's';
	if (mode == "showwhois")
		return 'W';
	if (mode == "snomask")
		return 's';
	if (mode == "sslonly")
		return 'z';
	if (mode == "stripcolor")
		return 'S';
	if (mode == "topiclock")
		return 't';
	if (mode == "u_censor")
		return 'G';
	if (mode == "u_registered")
		return 'r';
	if (mode == "u_stripcolor")
		return 'S';
	if (mode == "voice")
		return 'v';
	if (mode == "wallops")
		return 'w';
	return ' ';
}

std::string Client::convertChanMode(char mode) {
	switch (mode) {
		case 'a':
			return "admin";
		case 'b':
			return "ban";
		case 'c':
			return "blockcolor";
		case 'd':
			return "delaymsg";
		case 'e':
			return "banexception";
		case 'f':
			return "flood";
		case 'g':
			return "filter";
		case 'h':
			return "halfop";
		case 'i':
			return "inviteonly";
		case 'j':
			return "joinflood";
		case 'k':
			return "key";
		case 'l':
			return "limit";
		case 'm':
			return "moderated";
		case 'n':
			return "noextmsg";
		case 'o':
			return "op";
		case 'p':
			return "private";
		case 'q':
			return "founder";
		case 'r':
			return "c_registered";
		case 's':
			return "secret";
		case 't':
			return "topiclock";
		case 'u':
			return "auditorium";
		case 'v':
			return "voice";
		case 'w':
			return "autoop";
		case 'y':
			return "operprefix";
		case 'z':
			return "sslonly";
		case 'A':
			return "allowinvite";
		case 'B':
			return "blockcaps";
		case 'C':
			return "noctcp";
		case 'D':
			return "delayjoin";
		case 'F':
			return "nickflood";
		case 'G':
			return "censor";
		case 'H':
			return "history";
		case 'I':
			return "invex";
		case 'J':
			return "kicknorejoin";
		case 'K':
			return "noknock";
		case 'L':
			return "redirect";
		case 'M':
			return "regmoderated";
		case 'N':
			return "nonick";
		case 'O':
			return "operonly";
		case 'P':
			return "permanent";
		case 'Q':
			return "nokick";
		case 'R':
			return "reginvite";
		case 'S':
			return "stripcolor";
		case 'T':
			return "nonotice";
		case 'U':
			return "opmoderated";
		case 'W':
			return "chanacl";
		case 'X':
			return "exemptchanops";
		case 'Y':
			return "official-join";
		default:
			return "";
	}
}

std::string Client::convertUserMode(char mode) {
	switch (mode) {
		case 'c':
			return "deaf_commonchan";
		case 'd':
			return "deaf";
		case 'g':
			return "callerid";
		case 'i':
			return "invisible";
		case 'n':
			return "noautoop";
		case 'o':
			return "oper";
		case 'r':
			return "u_registered";
		case 's':
			return "snomask";
		case 'w':
			return "wallops";
		case 'x':
			return "cloak";
		case 'B':
			return "bot";
		case 'G':
			return "u_censor";
		case 'H':
			return "hideoper";
		case 'I':
			return "hidechans";
		case 'R':
			return "regdeaf";
		case 'S':
			return "u_stripcolor";
		case 'W':
			return "showwhois";
		default:
			return "";
	}
}

void Client::parse005(std::vector<std::string> parsedLine) {
	for (unsigned int i = 0; i < parsedLine.size(); i++) {
		if (parsedLine[i].size() > 8 && parsedLine.substr(0, 7) == "PREFIX=") {
			std::queue<char> prefixModes;
			unsigned int j = 8;
			for (; parsedLine[i][j] != ')'; j++)
				prefixModes.push(parsedLine[i][j]);
			for (j++; !prefixModes.empty(); j++) {
				statusPrefixes.push_back(std::pair<char, char> (prefixModes.front(), parsedLine[i][j]));
				prefixModes.pop();
			}
		} else if (parsedLine[i].size() > 10 && parsedLine.substr(0, 10) == "CHANMODES=") {
			std::vector<char> modes;
			for (unsigned int j = 10; j < parsedLine[i].size(); j++) {
				if (parsedLine[i][j] == ',') {
					chanModes.push_back(modes);
					modes.clear();
				} else
					modes.push_back(parsedLine[i][j]);
			}
			if (!modes.empty())
				chanModes.push_back(modes);
		} else if (parsedLine[i].size() > 10 && parsedLine.substr(0, 10) == "CHANTYPES=") {
			for (unsigned int j = 10; j < parsedLine[i].size(); j++)
				chanTypes.push_back(parsedLine[i][j]);
		} else if (parsedLine[i].size() > 6 && parsedLine.substr(0, 6) == "MODES=") {
			std::istringstream modeNum (parsedLine[i].substr(6));
			modeNum >> maxModes;
		}
	}
}

void Client::parseNames(std::string channel, std::string namesList) {
	bool firstNames = false;
	if (!readingNames[channel]) {
		for (std::list<std::string>::iterator nickIter = inChannels[channel].second.second.second.begin(); nickIter != inChannels[channel].second.second.second.end(); ++nickIter) {
			std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(*nickIter);
			if (userIter == users.end())
				continue;
			userIter->second->removeChannel(channel);
			if (userIter->second->channelList().empty()) {
				delete userIter->second;
				users.erase(userIter);
			}
		}
		inChannels[channel].second.second.second.clear();
		readingNames[channel] = firstNames = true;
	}
	std::vector<std::string> names;
	std::string name = "";
	for (size_t i = 0; i < namesList.size(); i++) {
		if (namesList[i] == ' ') {
			names.push_back(name);
			name = "";
		} else
			name += namesList[i];
	}
	if (name != "")
		names.push_back(name);
	for (unsigned int i = 0; i < names.size(); i++) {
		std::vector<char> rank;
		for (std::list<std::pair<char, char> >::iterator prefixIter = statusPrefixes.begin(); prefixIter != statusPrefixes.end(); ++prefixIter) {
			if ((*prefixIter).second == names[i][0]) {
				rank.push_back((*prefixIter).first);
				names[i] = names[i].substr(1);
			}
		}
		std::string nick, ident = "", host = "";
		size_t exclamation = names[i].find_first_of('!');
		if (exclamation == std::string::npos)
			nick = names[i];
		else {
			size_t at = names[i].find_first_of('@');
			nick = names[i].substr(0, exclamation);
			ident = names[i].substr(exclamation + 1, at - exclamation - 1);
			host = names[i].substr(at + 1);
		}
		std::tr1::unordered_map<std::string, User*>::iterator joiningUser = users.find(nick);
		if (joiningUser == users.end()) {
			users.insert(std::pair<std::string, User*> (nick, new User(ident, host)));
			joiningUser = users.find(nick);
		} else {
			joiningUser->second->ident(ident);
			joiningUser->second->host(host);
		}
		inChannels[channel].second.second.second.insert(nick);
		joiningUser->second->addChannel(channel);
		for (unsigned int i = 0; i < rank.size(); i++)
			joiningUser->second->status(channel, rank, true);
	}
	if (firstNames) {
		if (users.find(name)->second->ident() == "" || users.find(name)->second->host() == "")
			dataToSend.push("WHO " + channel);
	}
}

std::list<std::string> Client::clients() {
	std::list<std::string> ircClients;
	ircClients.push_back(serverConf["nick"]);
	return ircClients;
}

std::tr1::unordered_map<std::string, std::string> Client::clientInfo(std::string client) {
	std::tr1::unordered_map<std::string, std::string> info;
	info.insert(std::pair<std::string, std::string> ("nick", serverConf["nick"]));
	info.insert(std::pair<std::string, std::string> ("gecos", serverConf["gecos"]));
	std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(serverConf["nick"]);
	if (userIter == users.end())
		return info;
	info.insert(std::pair<std::string, std::string> ("ident", userIter->second->ident()));
	info.insert(std::pair<std::string, std::string> ("host", userIter->second->host()));
	return info;
}

std::list<std::string> Client::userModes(std::string client) {
	return uModes;
}

extern "C" Protocol* spawn() {
	return new Client;
}