#include "protoinclude.h"
#include <algorithm>
#include <queue>

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
		void status(std::string channel, std::string status, bool adding);
		std::string status(std::string channel);
	private:
		std::string userIdent, userHost;
		std::tr1::unordered_map<std::string, std::set<std::string> > channels;
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
		std::set<std::string> channelModes(std::string channel);
		std::string userIdent(std::string user);
		std::string userHost(std::string user);
		std::pair<std::string, char> userStatus(std::string channel, std::string user);
		std::string compareStatus(std::set<std::string> statuses);
		void sendMsg(std::string client, std::string target, std::string message);
		void sendNotice(std::string client, std::string target, std::string message);
		void setMode(std::string client, std::string target, std::list<std::string> addModes, std::list<std::string> remModes);
		void joinChannel(std::string client, std::string channel, std::string key = "");
		void partChannel(std::string client, std::string channel, std::string reason = "");
		void quitServer(std::string reason);
		void kickUser(std::string client, std::string channel, std::string user, std::string reason);
		void changeNick(std::string client, std::string newNick);
		void oper(std::string client, std::string username, std::string password);
		void sendOther(std::string rawLine);
		std::set<std::string> clients();
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
		std::tr1::unordered_map<std::string, char> allModes;
		std::tr1::unordered_map<char, std::string> allChanModes, allUserModes;
		void setChanMode(bool addMode, bool list, std::string channel, std::string mode, std::string param = "");
		void setStatus(bool addMode, std::string channel, std::string status, std::string user);
		void saveMode(std::string longmode, char modeChar, bool chanMode);
		char convertMode(std::string mode);
		std::string convertChanMode(char mode);
		std::string convertUserMode(char mode);
		void parse005(std::vector<std::string> parsedLine);
		void parseNames(std::string channel, std::string namesList);
		bool isNumeric(std::string command);
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
	channels.insert(std::pair<std::string, std::set<std::string> > (channel, std::set<std::string> ()));
}

void User::removeChannel(std::string channel) {
	channels.erase(channels.find(channel));
}

std::list<std::string> User::channelList() {
	std::list<std::string> listOfChannels;
	for (std::tr1::unordered_map<std::string, std::set<std::string> >::iterator chanIter = channels.begin(); chanIter != channels.end(); ++chanIter)
		listOfChannels.push_back(chanIter->first);
	return listOfChannels;
}

void User::status(std::string channel, std::string status, bool adding) {
	if (channels.find(channel) == channels.end())
		return;
	if (adding)
		channels.find(channel)->second.insert(status);
	else
		channels.find(channel)->second.erase(channels.find(channel)->second.find(status));
}

std::string User::status(std::string channel) {
	if (channels.find(channel) == channels.end())
		return "";
	std::set<std::string> statuses = channels.find(channel)->second;
	return server->compareStatus(statuses);
}

Client::Client(std::string serverAddress, std::tr1::unordered_map<std::string, std::string> confVars, Base* theBase, unsigned short debug) : Protocol(serverAddress, confVars, theBase, debug), maxModes(1) {
	pthread_mutex_init(&secondsmutex, NULL); // initialize mutex for use in sending threads
	pthread_attr_init(&detachedState);
	pthread_attr_setdetachstate(&detachedState, PTHREAD_CREATE_DETACHED);
	if (connection == NULL) {
		std::cout << "p_client: " << serverName << ": Socket handle could not be obtained." << std::endl;
		keepServer = false;
		return;
	}
	if (serverConf["bind"] != "") {
		if (!connection->bindSocket(serverConf["bind"]))
			std::cout << "Could not bind to " << serverConf["bind"] << "; trying without binding.  Abort RoBoBo and adjust configuration settings to try again with binding." << std::endl; // debug level 1
	}
	if (confVars["floodcontrol"][0] == 'n' || confVars["floodcontrol"][0] == 'N')
		floodControl = false;
	else
		floodControl = true;
	/* saveMode(long form, default char, whether it's a channel mode)
	 * This function allows repeated additions of modes to saved mode conversions
	 * without too much code duplication and allowing easy changes to mode customization
	 * if necessary.  The modes specified here represent the default list of long name
	 * modes and their associated mode letters.  If you are using this list as a reference,
	 * the list is alphebetized by the long names.
	 */
	saveMode("admin", 'a', true);
	saveMode("allowinvite", 'A', true);
	saveMode("auditorium", 'u', true);
	saveMode("autoop", 'w', true);
	saveMode("ban", 'b', true);
	saveMode("banexception", 'e', true);
	saveMode("blockcaps", 'B', true);
	saveMode("blockcolor", 'c', true);
	saveMode("bot", 'B', false);
	saveMode("c_registered", 'r', true);
	saveMode("callerid", 'g', false);
	saveMode("censor", 'G', true);
	saveMode("chanacl", 'W', true);
	saveMode("cloak", 'x', false);
	saveMode("deaf", 'd', false);
	saveMode("deaf_commonchan", 'c', false);
	saveMode("delayjoin", 'D', true);
	saveMode("delaymsg", 'd', true);
	saveMode("exemptchanops", 'X', true);
	saveMode("filter", 'g', true);
	saveMode("flood", 'f', true);
	saveMode("founder", 'q', true);
	saveMode("halfop", 'h', true);
	saveMode("hidechans", 'I', false);
	saveMode("hideoper", 'H', false);
	saveMode("history", 'H', true);
	saveMode("invex", 'I', true);
	saveMode("invisible", 'i', false);
	saveMode("inviteonly", 'i', true);
	saveMode("joinflood", 'j', true);
	saveMode("key", 'k', true);
	saveMode("kicknorejoin", 'J', true);
	saveMode("limit", 'l', true);
	saveMode("longmode", 'Z', true);
	saveMode("moderated", 'm', true);
	saveMode("nickflood", 'F', true);
	saveMode("noautoop", 'n', false);
	saveMode("noctcp", 'C', true);
	saveMode("noextmsg", 'n', true);
	saveMode("nokick", 'Q', true);
	saveMode("noknock", 'K', true);
	saveMode("nonick", 'N', true);
	saveMode("nonotice", 'T', true);
	saveMode("official-join", 'Y', true);
	saveMode("op", 'o', true);
	saveMode("oper", 'o', false);
	saveMode("operonly", 'O', true);
	saveMode("operprefix", 'y', true);
	saveMode("opmoderated", 'U', true);
	saveMode("permanent", 'P', true);
	saveMode("private", 'p', true);
	saveMode("redirect", 'L', true);
	saveMode("regdeaf", 'R', false);
	saveMode("reginvite", 'R', true);
	saveMode("regmoderated", 'M', true);
	saveMode("secret", 's', true);
	saveMode("showwhois", 'W', false);
	saveMode("snomask", 's', false);
	saveMode("sslonly", 'z', true);
	saveMode("stripcolor", 'S', true);
	saveMode("topiclock", 't', true);
	saveMode("u_censor", 'G', false);
	saveMode("u_registered", 'r', false);
	saveMode("u_stripcolor", 'S', false);
	saveMode("voice", 'v', true);
	saveMode("wallops", 'w', false);
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
	callPreConnectHook();
	std::istringstream portNumber (serverConf["port"]);
	unsigned short port;
	portNumber >> port;
	connection->connectServer(serverName, port);
	sleep(1); // don't send right away in case of some sort of death or slowness
	callConnectHook(serverConf["nick"]);
	if (serverConf["password"] != "")
		sendOther("PASS " + serverConf["password"]);
	changeNick("", serverConf["nick"]);
	sendOther("USER " + serverConf["ident"] + " here " + serverName + " :" + serverConf["gecos"]);
	pthread_create(&receiveThread, &detachedState, handleData_thread, this);
	pthread_create(&sendThread, &detachedState, sendData_thread, this);
}

bool Client::isClient() {
	return true;
}

std::list<std::pair<std::string, char> > Client::prefixes() {
	std::list<std::pair<std::string, char> > longPrefixes;
	for (std::list<std::pair<char, char> >::iterator statusIter = statusPrefixes.begin(); statusIter != statusPrefixes.end(); ++statusIter)
		longPrefixes.push_back(std::pair<std::string, char> (convertChanMode((*statusIter).first), (*statusIter).second));
	return longPrefixes;
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
	return chanIter->second.first;
}

std::set<std::string> Client::channelUsers(std::string channel) {
	std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > >::iterator chanIter = inChannels.find(channel);
	if (chanIter == inChannels.end())
		return std::set<std::string>();
	return chanIter->second.second.second;
}

std::set<std::string> Client::channelModes(std::string channel) {
	std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > >::iterator chanIter = inChannels.find(channel);
	if (chanIter == inChannels.end())
		return std::set<std::string>();
	std::set<std::string> modes;
	for (std::list<std::string>::iterator modeIter = chanIter->second.second.first.begin(); modeIter != chanIter->second.second.first.end(); ++modeIter)
		modes.insert(*modeIter);
	return modes;
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
		if ((*statIter).first == convertMode(status))
			return std::pair<std::string, char> (status, (*statIter).second);
	}
	return std::pair<std::string, char> ("", ' '); // Um, some bug perhaps?
}

std::string Client::compareStatus(std::set<std::string> statuses) {
	for (std::list<std::pair<char, char> >::iterator statIter = statusPrefixes.begin(); statIter != statusPrefixes.end(); ++statIter) {
		std::string thisStatus = convertChanMode(statIter->first);
		if (statuses.find(thisStatus) != statuses.end())
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

void Client::setMode(std::string client, std::string target, std::list<std::string> addModes, std::list<std::string> remModes) {
	std::string modeCommand = "MODE " + target + " ", addModeChars = "+", remModeChars = "-", params = "";
	if (addModes.size() > maxModes) {
		std::list<std::string> newAddModes = addModes;
		std::list<std::string>::iterator modeIter = addModes.begin(), newModeIter = newAddModes.begin();
		for (unsigned int i = 0; i < maxModes; i++) {
			++modeIter;
			++newModeIter;
		}
		newAddModes.erase(newAddModes.begin(), newModeIter);
		setMode(client, target, newAddModes, remModes);
		addModes.erase(modeIter, addModes.end());
		remModes.clear();
	} else if (addModes.size() + remModes.size() > maxModes) {
		unsigned int numModes = maxModes - addModes.size();
		std::list<std::string> newRemModes = remModes;
		std::list<std::string>::iterator modeIter= remModes.begin(), newModeIter = newRemModes.begin();
		for (unsigned int i = 0; i < numModes; i++) {
			++modeIter;
			++newModeIter;
		}
		newRemModes.erase(newRemModes.begin(), newModeIter);
		setMode(client, target, std::list<std::string> (), newRemModes);
		remModes.erase(modeIter, remModes.end());
	}
	for (std::list<std::string>::iterator modeIter = addModes.begin(); modeIter != addModes.end(); ++modeIter) {
		char modeChar = convertMode(*modeIter);
		if (modeChar == ' ' && convertMode("longmode") != ' ') {
			modeChar = convertMode("longmode");
			params += " " + *modeIter;
		} else if (modeChar == ' ')
			continue;
		else {
			if ((*modeIter).find_first_of('=') != std::string::npos)
				params += " " + (*modeIter).substr((*modeIter).find_first_of('=') + 1);
		}
		addModeChars += modeChar;
	}
	for (std::list<std::string>::iterator modeIter = remModes.begin(); modeIter != remModes.end(); ++modeIter) {
		char modeChar = convertMode(*modeIter);
		if (modeChar == ' ')
			continue;
		if ((*modeIter).find_first_of('=') != std::string::npos)
			params += " " + (*modeIter).substr((*modeIter).find_first_of('=') + 1);
		remModeChars += modeChar;
	}
	if (addModeChars == "+" && remModeChars == "-")
		return; // No modes passed here were good, so don't send blank mode command
	std::string modeString = "MODE " + target + " ";
	if (addModeChars != "+")
		modeString += addModeChars;
	if (remModeChars != "-")
		modeString += remModeChars;
	modeString += params;
	dataToSend.push(modeString);
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

void Client::oper(std::string client, std::string username, std::string password) {
	dataToSend.push("OPER " + username + " " + password);
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
		receivedLine = connection->receive();
		if (receivedLine == "")
			break; // this case indicates a receive error
		if (debugLevel >= 3)
			std::cout << receivedLine << std::endl;
		parsedLine = parseLine(receivedLine);
		if (parsedLine[1] == "001") { // welcome to the network
			if (allModes.find("bot") != allModes.end()) {
				std::list<std::string> botMode;
				botMode.push_back("bot");
				setMode(serverConf["nick"], serverConf["nick"], botMode, std::list<std::string> ()); // set bot mode
			}
			if (serverConf["channels"] != "")
				joinChannel("", serverConf["channels"]);
			registered = true;
			sendOther("WHOIS " + serverConf["nick"]);
			callNumericHook(serverConf["nick"], "001", parsedLine);
		} else if (parsedLine[1] == "005") { // server features
			parse005(parsedLine);
			callNumericHook(serverConf["nick"], "005", parsedLine);
		} else if (parsedLine[1] == "311" && parsedLine[3] == serverConf["nick"]) {
			serverConf["ident"] = parsedLine[4];
			serverConf["host"] = parsedLine[5];
			callNumericHook(serverConf["nick"], "311", parsedLine);
		} else if (parsedLine[1] == "324") { //:stitch.chatspike.net 324 Werewolf #robobo +nt
			inChannels.find(parsedLine[3])->second.second.first.clear();
			unsigned int currParam = 5;
			for (size_t i = 0; i < parsedLine[4].size(); i++) {
				if (parsedLine[4][i] == '+')
					continue;
				std::string mode = convertChanMode(parsedLine[4][i]);
				bool param = false;
				for (size_t j = 0; j < chanModes[0].size(); j++) {
					if (parsedLine[4][i] == chanModes[0][j]) {
						param = true;
						break;
					}
				}
				if (!param) {
					for (size_t j = 0; j < chanModes[1].size(); j++) {
						if (parsedLine[4][i] == chanModes[1][j]) {
							param = true;
							break;
						}
					}
				}
				if (!param) {
					for (size_t j = 0; j < chanModes[2].size(); j++) {
						if (parsedLine[4][i] == chanModes[2][j]) {
							param = true;
							break;
						}
					}
				}
				if (param)
					mode += "=" + parsedLine[currParam++];
				inChannels.find(parsedLine[3])->second.second.first.push_back(mode);
			}
			callNumericHook(serverConf["nick"], "324", parsedLine);
		} else if (parsedLine[1] == "332") { // channel topic
			std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > >::iterator it = inChannels.find(parsedLine[3]);
			if (it != inChannels.end())
				it->second.first = parsedLine[4];
			callNumericHook(serverConf["nick"], "332", parsedLine);
		} else if (parsedLine[1] == "352") { // WHO reply
			std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(parsedLine[7]);
			if (userIter != users.end()) {
				userIter->second->ident(parsedLine[4]);
				userIter->second->host(parsedLine[5]);
			}
			callNumericHook(serverConf["nick"], "352", parsedLine);
		} else if (parsedLine[1] == "353") { // NAMES reply
			parseNames(parsedLine[4], parsedLine[5]);
			callNumericHook(serverConf["nick"], "353", parsedLine);
		} else if (parsedLine[1] == "366") { // end of NAMES reply
			readingNames[parsedLine[3]] = false;
			callNumericHook(serverConf["nick"], "366", parsedLine);
		} else if (parsedLine[1] == "433" && !registered) { // nickname already in use
			if (!altChanged && serverConf["altnick"] != "") {
				changeNick("", serverConf["altnick"]);
				serverConf["nick"] = serverConf["altnick"];
				altChanged = true;
			} else
				quitServer("");
			callNumericHook(serverConf["nick"], "433", parsedLine);
		} else if (isNumeric(parsedLine[1]))
			callNumericHook(serverConf["nick"], parsedLine[1], parsedLine);
		else if (parsedLine[1] == "PRIVMSG") {
			if (chanTypes.find(parsedLine[2][0]) == chanTypes.end() && chanTypes.find(parsedLine[2][1]) == chanTypes.end()) { // user message
				if (parsedLine[3][0] == (char)1) {
					parsedLine[3] = parsedLine[3].substr(1);
					if (parsedLine[3][parsedLine[3].size() - 1] == (char)1)
						parsedLine[3] = parsedLine[3].substr(0, parsedLine[3].size() - 1);
					callUserCTCPHook(serverConf["nick"], parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1), parsedLine[3]);
				} else
					callUserMsgHook(serverConf["nick"], parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1), parsedLine[3]);
			} else { // channel message
				char status = ' ';
				if (chanTypes.find(parsedLine[2][0]) == chanTypes.end()) {
					status = parsedLine[2][0];
					parsedLine[2] = parsedLine[2].substr(1);
				}
				if (parsedLine[3][0] == (char)1) {
					parsedLine[3] = parsedLine[3].substr(1);
					if (parsedLine[3][parsedLine[3].size() - 1] == (char)1)
						parsedLine[3] = parsedLine[3].substr(0, parsedLine[3].size() - 1);
					callChannelCTCPHook(serverConf["nick"], parsedLine[2], status, parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1), parsedLine[3]);
				} else
					callChanMsgHook(serverConf["nick"], parsedLine[2], status, parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1), parsedLine[3]);
			}
		} else if (parsedLine[1] == "NOTICE") {
			if (chanTypes.find(parsedLine[2][0]) == chanTypes.end() && chanTypes.find(parsedLine[2][1]) == chanTypes.end()) { // user notice
				if (parsedLine[3][0] == (char)1) {
					parsedLine[3] = parsedLine[3].substr(1);
					if (parsedLine[3][parsedLine[3].size() - 1] == (char)1)
						parsedLine[3] = parsedLine[3].substr(0, parsedLine[3].size() - 1);
					callUserCTCPReplyHook(serverConf["nick"], parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1), parsedLine[3]);
				} else
					callUserNoticeHook(serverConf["nick"], parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1), parsedLine[3]);
			} else { // channel notice
				char status = ' ';
				if (chanTypes.find(parsedLine[2][0]) == chanTypes.end()) {
					status = parsedLine[2][0];
					parsedLine[2] = parsedLine[2].substr(1);
				}
				if (parsedLine[3][0] == (char)1) {
					parsedLine[3] = parsedLine[3].substr(1);
					if (parsedLine[3][parsedLine[3].size() - 1] == (char)1)
						parsedLine[3] = parsedLine[3].substr(0, parsedLine[3].size() - 1);
					callChannelCTCPReplyHook(serverConf["nick"], parsedLine[2], status, parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1), parsedLine[3]);
				} else
					callChanNoticeHook(serverConf["nick"], parsedLine[2], status, parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1), parsedLine[3]);
			}
		} else if (parsedLine[1] == "MODE") {
			bool addMode = true;
			if (parsedLine[2] == serverConf["nick"]) { // if it's a user mode
				for (unsigned int i = 0; i < parsedLine[3].size(); i++) {
					if (parsedLine[3][i] == '+')
						addMode = true;
					else if (parsedLine[3][i] == '-')
						addMode = false;
					else {
						std::string longmode = convertUserMode(parsedLine[3][i]);
						callUserModePreHook(serverConf["nick"], longmode, addMode);
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
						callUserModePostHook(serverConf["nick"], longmode, addMode);
					}
				}
			} else { // it's a channel mode
				size_t currParam = 4; // MODE parameters start at the fourth IRC parameter
				for (size_t i = 0; i < parsedLine[3].size(); i++) {
					if (parsedLine[3][i] == '+')
						addMode = true;
					else if (parsedLine[3][i] == '-')
						addMode = false;
					else {
						int category;
						bool found = false, prefix = false;
						for (std::list<std::pair<char, char> >::iterator statusIter = statusPrefixes.begin(); statusIter != statusPrefixes.end(); ++statusIter) {
							if ((*statusIter).first == parsedLine[3][i]) {
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
						std::string setterNick = parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1);
						std::string longmode = convertChanMode(parsedLine[3][i]);
						if (longmode == "longmode") {
							longmode = parsedLine[currParam].substr(0, parsedLine[currParam].find_first_of('='));
							parsedLine[currParam] = parsedLine[currParam].substr(parsedLine[currParam].find_first_of('=') + 1);
						}
						if (category == 0 || category == 1 || (category == 2 && addMode))
							callChannelModePreHook(parsedLine[2], setterNick, longmode, addMode, parsedLine[currParam]);
						else
							callChannelModePreHook(parsedLine[2], setterNick, longmode, addMode);
						if (category == 0) {
							std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > >::iterator chanIter = inChannels.find(parsedLine[2]);
							if (chanIter != inChannels.end()) {
								if (prefix)
									setStatus(addMode, chanIter->first, convertChanMode(parsedLine[3][i]), parsedLine[currParam]);
								else
									setChanMode(addMode, true, chanIter->first, convertChanMode(parsedLine[3][i]), parsedLine[currParam]);
							}
						} else if (category == 1 || (category == 2 && addMode)) {
							std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > >::iterator chanIter = inChannels.find(parsedLine[2]);
							if (chanIter != inChannels.end())
								setChanMode(addMode, false, chanIter->first, convertChanMode(parsedLine[3][i]), parsedLine[currParam]);
						} else {
							std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > >::iterator chanIter = inChannels.find(parsedLine[2]);
							if (chanIter != inChannels.end())
								setChanMode(addMode, false, chanIter->first, convertChanMode(parsedLine[3][i]));
						}
						if (category == 0 || category == 1 || (category == 2 && addMode))
							callChannelModePostHook(parsedLine[2], setterNick, longmode, addMode, parsedLine[currParam++]); // increase parameter now that we're done with the current one
						else
							callChannelModePostHook(parsedLine[2], setterNick, longmode, addMode);
					}
				}
			}
		} else if (parsedLine[1] == "NICK" && serverConf["nick"] == parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1)) { // bot's nick changed
			callNickChangePreHook(serverConf["nick"], parsedLine[2]);
			std::string oldNick = serverConf["nick"];
			serverConf["nick"] = parsedLine[2];
			callNickChangePostHook(oldNick, parsedLine[2]);
		} else if (parsedLine[1] == "NICK") {
			std::string oldNick = parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1);
			callNickChangePreHook(oldNick, parsedLine[2]);
			for (std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > >::iterator chanIter = inChannels.begin(); chanIter != inChannels.end(); ++chanIter) {
				if (chanIter->second.second.second.find(oldNick) != chanIter->second.second.second.end()) {
					chanIter->second.second.second.erase(oldNick);
					chanIter->second.second.second.insert(parsedLine[2]);
				}
			}
			User* nickChanger = users.find(oldNick)->second;
			users.erase(oldNick);
			users.insert(std::pair<std::string, User*> (parsedLine[2], nickChanger));
			callNickChangePostHook(oldNick, parsedLine[2]);
		} else if (parsedLine[1] == "JOIN" && serverConf["nick"] == parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1)) { // bot joined a channel
			callChannelJoinPreHook(parsedLine[2], parsedLine[0].substr(1));
			inChannels.insert(std::pair<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > > (parsedLine[2], std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > ()));
			callChannelJoinPostHook(parsedLine[2], parsedLine[0].substr(1));
		} else if (parsedLine[1] == "JOIN") {
			std::string hostmask = parsedLine[0].substr(1);
			callChannelJoinPreHook(parsedLine[2], hostmask);
			std::string nick = hostmask.substr(0, hostmask.find_first_of('!'));
			if (users.find(nick) == users.end()) {
				size_t exclaim = hostmask.find_first_of('!');
				std::string ident = hostmask.substr(exclaim, hostmask.find_first_of('@') - exclaim);
				std::string host = hostmask.substr(hostmask.find_first_of('@'));
				users.insert(std::pair<std::string, User*> (nick, new User(ident, host, this)));
			}
			inChannels.find(parsedLine[2])->second.second.second.insert(nick);
			users.find(nick)->second->addChannel(parsedLine[2]);
			callChannelJoinPostHook(parsedLine[2], hostmask);
		} else if (parsedLine[1] == "PART" && serverConf["nick"] == parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1)) {
			if (parsedLine.size() == 3)
				callChannelPartPreHook(parsedLine[2], parsedLine[0].substr(1));
			else
				callChannelPartPreHook(parsedLine[2], parsedLine[0].substr(1), parsedLine[3]);
			inChannels.erase(parsedLine[2]);
			if (parsedLine.size() == 3)
				callChannelPartPostHook(parsedLine[2], parsedLine[0].substr(1));
			else
				callChannelPartPostHook(parsedLine[2], parsedLine[0].substr(1), parsedLine[3]);
		} else if (parsedLine[1] == "PART") {
			if (parsedLine.size() == 3)
				callChannelPartPreHook(parsedLine[2], parsedLine[0].substr(1));
			else
				callChannelPartPreHook(parsedLine[2], parsedLine[0].substr(1), parsedLine[3]);
			std::string nick = parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1);
			inChannels.find(parsedLine[2])->second.second.second.erase(nick);
			users.find(nick)->second->removeChannel(parsedLine[2]);
			if (inChannels.find(parsedLine[2])->second.second.second.empty())
				inChannels.erase(inChannels.find(parsedLine[2]));
			if (parsedLine.size() == 3)
				callChannelPartPostHook(parsedLine[2], parsedLine[0].substr(1));
			else
				callChannelPartPostHook(parsedLine[2], parsedLine[0].substr(1), parsedLine[3]);
		} else if (parsedLine[1] == "QUIT" && serverConf["nick"] == parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1)) {
			callQuitHook(serverConf["nick"]);
			connection->closeConnection();
			quitHooked = true;
			keepServer = false;
			break;
		} else if (parsedLine[1] == "QUIT") {
			callUserQuitPreHook(parsedLine[0].substr(1), parsedLine[2]);
			std::string nick = parsedLine[0].substr(1, parsedLine[0].find_first_of('!') - 1);
			for (std::tr1::unordered_map<std::string, std::pair<std::string, std::pair<std::list<std::string>, std::set<std::string> > > >::iterator chanIter = inChannels.begin(); chanIter != inChannels.end(); ++chanIter)
				chanIter->second.second.second.erase(nick);
			std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(nick);
			delete userIter->second;
			users.erase(userIter);
			callUserQuitPostHook(parsedLine[0].substr(1), parsedLine[2]);
		} else if (parsedLine[1] == "KILL" && serverConf["nick"] == parsedLine[2]) {
			callQuitHook(serverConf["nick"]);
			connection->closeConnection();
			quitHooked = true;
			keepServer = false;
			break;
		} else if (parsedLine[0] == "PING") { // server ping
			sendOther("PONG " + parsedLine[1]);
			callOtherDataHook(serverConf["nick"], parsedLine);
		} else
			callOtherDataHook(serverConf["nick"], parsedLine);
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
	if (floodControl)
		pthread_create(&secondsThread, &detachedState, secondsDecrement_thread, this);
	while (true) {
		if (!connection->isConnected())
			break; // Thread must die when server isn't connected.
		if (dataToSend.empty()) {
			usleep(100000); // sleep for a short time to avoid processor abuse while being ready for data to arrive
			continue; // check again for empty queue
		}
		sendingMessage = dataToSend.front();
		dataToSend.pop();
		parsedLine = parseLine(sendingMessage);
		command = parsedLine[0];
		if (command == "PRIVMSG") {
			std::string message = parsedLine[2];
			bool channel = false, ctcp = false;
			char status = ' ';
			std::string targetName = parsedLine[1];
			if (chanTypes.find(targetName[0]) == chanTypes.end() && chanTypes.find(targetName[1]) == chanTypes.end()) { // User message!
				if (message[0] == (char)1) {
					ctcp = true;
					message = message.substr(1);
					if (message[message.size() - 1] == (char)1)
						message = message.substr(0, message.size() - 1);
					message = (char)1 + callUserCTCPOutHook(serverConf["nick"], targetName, message) + (char)1;
				} else
					message = callUserMessageOutHook(serverConf["nick"], targetName, message);
			} else { // Channel message!
				channel = true;
				if (chanTypes.find(targetName[0]) == chanTypes.end()) {
					status = targetName[0];
					targetName = targetName.substr(1);
				}
				if (message[0] == (char)1) {
					ctcp = true;
					message = message.substr(1);
					if (message[message.size() - 1] == (char)1)
						message = message.substr(0, message.size() - 1);
					message = (char)1 + callChannelCTCPOutHook(serverConf["nick"], targetName, status, message) + (char)1;
				} else
					message = callChannelMessageOutHook(serverConf["nick"], targetName, status, message);
			}
			if (message == "")
				continue; // This message clearly cannot be sent.
			sendingMessage = "PRIVMSG " + parsedLine[1] + " :" + message;
			parsedLine = parseLine(sendingMessage);
			if (ctcp && channel)
				callChannelCTCPSendHook(serverConf["nick"], targetName, status, message);
			else if (ctcp)
				callUserCTCPSendHook(serverConf["nick"], targetName, message);
			else if (channel)
				callChannelMessageSendHook(serverConf["nick"], targetName, status, message);
			else
				callUserMessageSendHook(serverConf["nick"], targetName, message);
		} else if (command == "NOTICE") {
			std::string message = parsedLine[2];
			bool channel = false, ctcp = false;
			char status = ' ';
			std::string targetName = parsedLine[1];
			if (chanTypes.find(targetName[0]) == chanTypes.end() && chanTypes.find(targetName[1]) == chanTypes.end()) { // User message!
				if (parsedLine[2][0] == (char)1) {
					message = parsedLine[2].substr(1);
					if (message[message.size() - 1] == (char)1)
						message = message.substr(0, message.size() - 1);
					message = (char)1 + callUserCTCPReplyOutHook(serverConf["nick"], targetName, message) + (char)1;
				} else
					message = callUserNoticeOutHook(serverConf["nick"], targetName, message);
			} else { // Channel message!
				if (chanTypes.find(targetName[0]) == chanTypes.end()) {
					status = targetName[0];
					targetName = targetName.substr(1);
				}
				if (parsedLine[2][0] == (char)1) {
					message = parsedLine[2].substr(1);
					if (message[message.size() - 1] == (char)1)
						message = message.substr(0, message.size() - 1);
					message = (char)1 + callChannelCTCPReplyOutHook(serverConf["nick"], targetName, status, message);
				} else
					message = callChannelNoticeOutHook(serverConf["nick"], targetName, status, message);
			}
			if (message == "")
				continue; // This message clearly cannot be sent.  So it won't.  Next line, please!
			sendingMessage = "NOTICE " + parsedLine[1] + " :" + parsedLine[2];
			parsedLine = parseLine(sendingMessage);
			if (channel && ctcp)
				callChannelCTCPReplySendHook(serverConf["nick"], targetName, status, message);
			else if (ctcp)
				callUserCTCPReplySendHook(serverConf["nick"], targetName, message);
			else if (channel)
				callChannelNoticeSendHook(serverConf["nick"], targetName, status, message);
			else
				callUserNoticeSendHook(serverConf["nick"], targetName, message);
		}
		if (command == "MODE") {
			secondsToAdd = 1;
			if (chanTypes.find(parsedLine[1][0]) == chanTypes.end() && parsedLine[2].find_first_of(convertMode("cloak")) != std::string::npos) // The usermode for cloaking
				secondsToAdd = 6; // Because apparently cloaking/setting hosts is so resource-intensive.
		} else { // MODE processes its own penalty addition
			if (command == "GLINE" || command == "KLINE" || (command == "NICK" && !registered) || command == "PASS" || command == "PING" || command == "PONG" || command == "QLINE" || command == "USER" || command == "ZLINE" || command == "OJOIN" || command == "SAJOIN" || command == "SAKICK" || command == "SAMODE" || command == "SANICK" || command == "SAPART" || command == "SAQUIT" || command == "SATOPIC")
				secondsToAdd = 0;
			else if (command == "MAP" || command == "REHASH" || command == "TOPIC" || command == "WHO" || command == "WHOIS" || command == "WHOWAS")
				secondsToAdd = 2;
			else if (command == "JOIN" || command == "CYCLE") // JOIN gets an extra second so we can also send MODE
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
		while (floodControl && seconds + secondsToAdd > 10)
			sleep(1);
		connection->sendData(sendingMessage);
		if (debugLevel >= 3)
			std::cout << " -> " << sendingMessage << std::endl;
		if (command == "JOIN") { // get channel modes
			connection->sendData("MODE " + parsedLine[1]);
			if (debugLevel >= 3)
				std::cout << " -> MODE " << parsedLine[1] << std::endl;
		}
		if (command == "QUIT") {
			connection->closeConnection();
			keepServer = false;
			break;
		}
		if (floodControl) {
			pthread_mutex_lock(&secondsmutex);
			seconds += secondsToAdd;
			pthread_mutex_unlock(&secondsmutex);
		}
	}
}

void* Client::secondsDecrement_thread(void* ptr) {
	Client* protomod = (Client*) ptr;
	protomod->secondsDecrement();
	return NULL;
}

void Client::secondsDecrement() {
	while (true) {
		if (!connection->isConnected()) {
			if (!quitHooked)
				callQuitHook(serverName);
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
			chanIter->second.second.first.push_back(mode + "=" + param);
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
			chanIter->second.second.first.push_back(mode);
		else
			chanIter->second.second.first.push_back(mode + "=" + param);
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
	if (rankedUser == users.end())
		return;
	rankedUser->second->status(channel, status, addMode);
}

void Client::saveMode(std::string longmode, char modeChar, bool chanMode) {
	if (serverConf[longmode] == "no" || serverConf[longmode] == "disable")
		return;
	if (serverConf[longmode] == "") {
		allModes.insert(std::pair<std::string, char> (longmode, modeChar));
		if (chanMode)
			allChanModes.insert(std::pair<char, std::string> (modeChar, longmode));
		else
			allUserModes.insert(std::pair<char, std::string> (modeChar, longmode));
		return;
	}
	allModes.insert(std::pair<std::string, char> (longmode, serverConf[longmode][0]));
	if (chanMode)
		allChanModes.insert(std::pair<char, std::string> (serverConf[longmode][0], longmode));
	else
		allUserModes.insert(std::pair<char, std::string> (serverConf[longmode][0], longmode));
}

char Client::convertMode(std::string mode) {
	if (mode.find_first_of('=') != std::string::npos)
		mode = mode.substr(0, mode.find_first_of('='));
	std::tr1::unordered_map<std::string, char>::iterator modeIter = allModes.find(mode);
	if (modeIter == allModes.end())
		return ' ';
	return modeIter->second;
}

std::string Client::convertChanMode(char mode) {
	std::tr1::unordered_map<char, std::string>::iterator modeIter = allChanModes.find(mode);
	if (modeIter == allChanModes.end())
		return "";
	return modeIter->second;
}

std::string Client::convertUserMode(char mode) {
	std::tr1::unordered_map<char, std::string>::iterator modeIter = allUserModes.find(mode);
	if (modeIter == allUserModes.end())
		return "";
	return modeIter->second;
}

void Client::parse005(std::vector<std::string> parsedLine) {
	for (unsigned int i = 0; i < parsedLine.size(); i++) {
		if (parsedLine[i].size() > 8 && parsedLine[i].substr(0, 7) == "PREFIX=") {
			std::queue<char> prefixModes;
			unsigned int j = 8;
			for (; parsedLine[i][j] != ')'; j++)
				prefixModes.push(parsedLine[i][j]);
			for (j++; !prefixModes.empty(); j++) {
				statusPrefixes.push_back(std::pair<char, char> (prefixModes.front(), parsedLine[i][j]));
				prefixModes.pop();
			}
		} else if (parsedLine[i].size() > 10 && parsedLine[i].substr(0, 10) == "CHANMODES=") {
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
		} else if (parsedLine[i].size() > 10 && parsedLine[i].substr(0, 10) == "CHANTYPES=") {
			for (unsigned int j = 10; j < parsedLine[i].size(); j++)
				chanTypes.insert(parsedLine[i][j]);
		} else if (parsedLine[i].size() > 6 && parsedLine[i].substr(0, 6) == "MODES=") {
			std::istringstream modeNum (parsedLine[i].substr(6));
			modeNum >> maxModes;
		}
	}
}

void Client::parseNames(std::string channel, std::string namesList) {
	bool firstNames = false;
	if (!readingNames[channel]) {
		for (std::set<std::string>::iterator nickIter = inChannels[channel].second.second.begin(); nickIter != inChannels[channel].second.second.end(); ++nickIter) {
			std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(*nickIter);
			if (userIter == users.end())
				continue;
			userIter->second->removeChannel(channel);
			if (userIter->second->channelList().empty()) {
				delete userIter->second;
				users.erase(userIter);
			}
		}
		inChannels[channel].second.second.clear();
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
		std::vector<std::string> rank;
		for (std::list<std::pair<char, char> >::iterator prefixIter = statusPrefixes.begin(); prefixIter != statusPrefixes.end(); ++prefixIter) {
			if ((*prefixIter).second == names[i][0]) {
				rank.push_back(convertChanMode((*prefixIter).first));
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
			users.insert(std::pair<std::string, User*> (nick, new User(ident, host, this)));
			joiningUser = users.find(nick);
		} else {
			joiningUser->second->ident(ident);
			joiningUser->second->host(host);
		}
		inChannels[channel].second.second.insert(nick);
		joiningUser->second->addChannel(channel);
		for (size_t i = 0; i < rank.size(); i++)
			joiningUser->second->status(channel, rank[i], true);
		name = nick; // for check below; doesn't really matter which nick it is.
	}
	if (firstNames) {
		if (users.find(name)->second->ident() == "" || users.find(name)->second->host() == "")
			dataToSend.push("WHO " + channel);
	}
}

std::set<std::string> Client::clients() {
	std::set<std::string> ircClients;
	ircClients.insert(serverConf["nick"]);
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

bool Client::isNumeric(std::string command) {
	if (command.size() != 3)
		return false;
	if (command[0] >= '0' && command[0] <= '9' && command[1] >= '0' && command[1] <= '9' && command[2] >= '0' && command[2] <= '9')
		return true;
	return false;
}

PROTOCOL_SPAWN(Client)