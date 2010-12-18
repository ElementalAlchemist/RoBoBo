#include "protoinclude.h"
#include <algorithm>

class Client;
class User {
	public:
		User(std::string ident, std::string host);
		void ident(std::string ident);
		std::string ident();
		void host(std::string host);
		std::string host();
		void addChannel(std::string channel);
		void removeChannel(std::string channel);
		std::list<std::string> channelList();
		void status(std::string channel, char status, bool adding);
		char status(std::string channel);
	private:
		std::string userIdent, userHost;
		std::tr1::unordered_map<std::string, std::set<char> > channels;
};
class Client : public Protocol {
	public:
		Client(std::string serverAddress, std::tr1::unordered_map<std::string, std::string> confVars, Base* theBase, unsigned short debug);
		~Client();
		unsigned int apiVersion();
		void connectServer();
		bool isClient();
		void sendMsg(std::string target, std::string message);
		void sendNotice(std::string target, std::string message);
		void setMode(std::string target, std::string mode);
		void removeMode(std::string target, std::string mode);
		void joinChannel(std::string channel, std::string key = "");
		void partChannel(std::string channel, std::string reason = "");
		void quitServer(std::string reason);
		void kickUser(std::string channel, std::string user, std::string reason);
		void changeNick(std::string newNick);
		void sendOther(std::string rawLine);
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
		std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > > channels;
		std::tr1::unordered_map<std::string, bool> readingNames;
		std::list<std::string> userModes;
		std::list<std::pair<char, char> > prefixes;
		std::vector<std::vector<char> > chanModes;
		void setChanMode(bool addMode, bool list, std::string channel, std::string mode, std::string param = "");
		void setStatus(bool addMode, std::string channel, std::string status, std::string user);
		char convertChanMode(std::string mode);
		std::string convertChanMode(char mode);
		char convertUserMode(std::string mode);
		std::string convertUserMode(char mode);
		std::vector<std::string> parseLine(std::string rawLine);
		void parse005(std::vector<std::string> parsedLine);
		void parseNames(std::string channel, std::string namesList);
};

User::User(std::string ident, std::string host) : userIdent(ident), userHost(host) {}

void User::ident(std::string ident) {
	userIdent = ident;
}

std::string User::ident() {
	return userIdent;
}

void User::host(std::string host) {
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
	return channels.find(channel)->second;
}

Client::Client(std::string serverAddress, std::tr1::unordered_map<std::string, std::string> confVars, Base* theBase, unsigned short debug) : Protocol(serverAddress, confVars, theBase, debug) {
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

void Client::sendMsg(std::string target, std::string message) {
	dataToSend.push("PRIVMSG " + target + " :" + message);
}

void Client::sendNotice(std::string target, std::string message) {
	dataToSend.push("NOTICE " + target + " :" + message);
}

void Client::setMode(std::string target, std::string mode);
void Client::removeMode(std::string target, std::string mode);

void Client::joinChannel(std::string channel, std::string key) {
	if (key == "")
		dataToSend.push("JOIN " + channel);
	else
		dataToSend.push("JOIN " + channel + " " + key);
}

void Client::partChannel(std::string channel, std::string reason) {
	dataToSend.push("PART " + channel + " :" + reason);
}

void Client::quitServer(std::string reason) {
	dataToSend.push("QUIT :" + reason);
}

void Client::kickUser(std::string channel, std::string user, std::string reason) {
	dataToSend.push("KICK " + channel + " " + user + " :" + reason);
}

void Client::changeNick(std::string newNick) {
	dataToSend.push("NICK " + newNick);
}

void Client::sendOther(std::string rawLine) {
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
		parsedLine = parseLine(receivedLine);
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
			std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > >::iterator it = channels.find(parsedLine[3]);
			if (it != channels.end())
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
							userModes.push_back(convertUserMode(parsedLine[3][i]));
						else {
							for (std::list<std::string>::iterator uModeIter = userModes.begin(); uModeIter != userModes.end(); ++uModeIter) {
								if (convertUserMode(parsedLine[3][i]) == *uModeIter) {
									userModes.erase(uModeIter);
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
						for (unsigned int j = 0; j < prefixes.size(); j++) {
							if (prefixes[j].first == parsedLine[3][i]) {
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
							std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > >::iterator chanIter = channels.find(parsedLine[2]);
							if (chanIter != channels.end()) {
								if (prefix)
									setStatus(addMode, chanIter->first, convertChanMode(parsedLine[3][i]), parsedLine[currParam++]);
								else
									setChanMode(addMode, true, chanIter->first, convertChanMode(parsedLine[3][i]), parsedLine[currParam]);
							}
						} else if (category == 1 || (category == 2 && addMode)) {
							std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > >::iterator chanIter = channels.find(parsedLine[2]);
							if (chanIter != channels.end())
								setChanMode(addMode, false, chanIter->first, convertChanMode(parsedLine[3][i]), parsedLine[currParam++]);
						} else {
							std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > >::iterator chanIter = channels.find(parsedLine[2]);
							if (chanIter != channels.end())
								setChanMode(addMode, false, chanIter->first, convertChanMode(parsedLine[3][i]));
						}
					}
				}
			}
		} else if (parsedLine[1] == "NICK" && serverConf["nick"] == parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1)) // bot's nick changed
			serverConf["nick"] = parsedLine[2];
		else if (parsedLine[1] == "NICK") {
			std::string oldNick = parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1);
			for (std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > >::iterator chanIter = channels.begin(); chanIter != channels.end(); ++chanIter) {
				if (chanIter->second.second.second.find(oldNick) != chanIter->second.second.second.end()) {
					chanIter->second.second.second.erase(oldNick);
					chanIter->second.second.second.insert(parsedLine[2]);
				}
			}
			User* nickChanger = users.find(oldNick)->second;
			users.erase(oldNick);
			users.insert(std::pair<std::string, User*> (parsedLine[2], nickChanger));
		} else if (parsedLine[1] == "JOIN" && serverConf["nick"] == parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1)) { // bot joined a channel
			channels.insert(std::pair<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > > (parsedLine[2], std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > ()));
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
			channels.find(parsedLine[2])->second.second.second.insert(nick);
		} else if (parsedLine[1] == "PART" && serverConf["nick"] == parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1))
			channels.erase(parsedLine[2]);
		else if (parsedLine[1] == "PART")
			channels.find(parsedLine[2])->second.second.second.erase(parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1));
		else if (parsedLine[1] == "QUIT" && serverConf["nick"] == parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1)) {
			connection.closeConnection();
			quitHooked = true;
			keepServer = false;
			break;
		} else if (parsedLine[1] == "QUIT") {
			for (std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > >::iterator chanIter = channels.begin(); chanIter != channels.end(); ++chanIter)
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
		parsedLine = parseLine(sendingMessage);
		command = parsedLine[0];
		if (command == "PRIVMSG" || command == "NOTICE") {
			std::string newMessage = botBase->callHookOut(serverName, parsedLine);
			if (newMessage == "")
				continue; // do not send this canceled message
			sendingMessage = parsedLine[0] + " " + parsedLine[1] + " :" + newMessage;
			parsedLine = parseLine(sendingMessage);
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
	std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > >::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
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

char Client::convertChanMode(std::string mode) {
	
}

std::string Client::convertChanMode(char mode) {
	
}

char Client::convertUserMode(std::string mode) {
	
}

std::string Client::convertUserMode(char mode) {
	
}

std::vector<std::string> Client::parseLine(std::string rawLine) {
	std::vector<std::string> parsedLine;
	std::string linePart = "";
	for (unsigned int i = 0; i < rawLine.size(); i++) {
		if (i != 0 && rawLine[i] == ':' && rawLine[i-1] == ' ') {
			while (i < rawLine.size())
				linePart += rawLine[i];
			parsedLine.push_back(linePart);
			return parsedLine;
		}
		if (rawLine[i] == ' ') {
			parsedLine.push_back(linePart);
			linePart = "";
			continue;
		}
		linePart += rawLine[i];
	}
	if (linePart != "")
		parsedLine.push_back(linePart);
	return parsedLine;
}

void Client::parse005(std::vector<std::string> parsedLine) {
	
}

void Client::parseNames(std::string channel, std::string namesList) {
	if (!readingNames[channel]) {
		for (std::list<std::string>::iterator nickIter = channels[channel].second.second.second.begin(); nickIter != channels[channel].second.second.second.end(); ++nickIter) {
			std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(*nickIter);
			if (userIter == users.end())
				continue;
			userIter->second->removeChannel(channel);
			if (userIter->second->channelList().empty()) {
				delete userIter->second;
				users.erase(userIter);
			}
		}
		channels[channel].second.second.second.clear();
		readingNames[channel] = true;
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
		for (std::list<std::pair<char, char> >::iterator prefixIter = prefixes.begin(); prefixIter != prefixes.end(); ++prefixIter) {
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
		channels[channel].second.second.second.insert(nick);
		joiningUser->second->addChannel(channel);
		for (unsigned int i = 0; i < rank.size(); i++)
			joiningUser->second->status(channel, rank, true);
	}
}

extern "C" Protocol* spawn() {
	return new Client;
}