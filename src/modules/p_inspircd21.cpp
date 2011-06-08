#include "protoinclude.h"
#include <ctime>

class InspIRCd;
class User {
	public:
		User(std::string theNick, std::string theIdent, std::string theHost, std::string theGecos, time_t theConnectTime);
		std::string nick();
		void nick(std::string newNick);
		std::string ident();
		void ident(std::string newIdent);
		std::string host();
		void host(std::string newHost);
		std::string gecos();
		void gecos(std::string newGecos);
		time_t nickTime();
		void updateTime(time_t time);
		std::string opertype();
		void operup(std::string opertype);
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
		void changeMetadata(std::string key, std::string value);
		std::string seeMetadata(std::string key);
	private:
		std::string userNick, userIdent, userHost, GECOS, oper;
		time_t nickTime;
		std::set<std::string> userModes;
		std::set<char> SNOMasks;
		std::tr1::unordered_map<std::string, std::string> metadata;
		std::tr1::unordered_map<std::string, std::set<std::string> > inChannels;
};

class Channel {
	public:
		Channel(time_t creation);
		std::set<std::string> modes();
		void addMode(std::string mode, bool list);
		void removeMode(std::string mode);
		std::set<std::string> users();
		void joinUser(std::string user);
		void partUser(std::string user);
		void joinUsers(std::set<std::string> users);
		std::string topic();
		void topic(std::string newTopic, time_t tTime);
		time_t topicSetTime();
		time_t creationTime();
		void changeMetadata(std::string key, std::string value);
		std::string seeMetadata(std::string key);
	private:
		std::set<std::string> chanModes, chanUsers;
		std::tr1::unordered_map<std::string, std::string> modeParams;
		std::string chanTopic;
		time_t createTime, topicTime;
		std::tr1::unordered_map<std::string, std::string> metadata;
};

class InspIRCd : public Protocol {
	public:
		InspIRCd(std::string serverAddr, std::tr1::unordered_map<std::string, std::string> config, Base* base, unsigned short debug);
		unsigned int apiVersion() = 0;
		void connectServer();
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
		void oper(std::string client, std::string username, std::string password);
		void killUser(std::string client, std::string user, std::string reason);
		void setXLine(std::string client, char lineType, std::string hostmask, time_t duration, std::string reason);
		void removeXLine(std::string client, char lineType, std::string hostmask);
		std::tr1::unordered_map<char, std::tr1::unordered_map<std::string, time_t> > listXLines();
		void sendSNotice(char snomask, std::string text);
		void sendOther(std::string rawLine);
		void addClient(std::string nick, std::string ident, std::string host, std::string gecos);
		void removeClient(std::string client, std::string reason);
		std::list<std::string> clients();
		std::tr1::unordered_map<std::string, std::string> clientInfo(std::string client);
		std::list<std::string> userModes(std::string client);
	private:
		pthread_t receiveThread;
		pthread_attr_t detachedState;
		static void* receiveData_thread(void* ptr);
		void receiveData();
		std::string connectedSID;
		std::set<std::string> ourClients;
		std::tr1::unordered_map<std::string, User*> users;
		std::tr1::unordered_map<std::string, std::string> nicks;
		std::tr1::unordered_map<std::string, Channel*> chans;
		std::list<std::pair<std::string, char> > chanRanks;
		std::vector<std::vector<std::string> > chanModes;
		std::tr1::unordered_map<std::string, char> allModes;
		std::tr1::unordered_map<char, std::string> allChanModes;
		std::tr1::unordered_map<char, std::string> allUserModes;
		char convertMode(std::string mode);
		std::string convertChanMode(char mode);
		std::string convertUserMode(char mode);
		std::tr1::unordered_map<char, std::tr1::unordered_map<std::string, time_t> > xLines;
		std::string uidCount;
		std::string useUID();
};

User::User(std::string theNick, std::string theIdent, std::string theHost, std::string theGecos, time_t theConnectTime) : userNick(theNick), userIdent(theIdent), userHost(theHost), GECOS(theGecos), nickTime(theConnectTime) {}

std::string User::nick() {
	return userNick;
}

void User::nick(std::string newNick) {
	userNick = newNick;
}

std::string User::ident() {
	return userIdent;
}

void User::ident(std::string newIdent) {
	userIdent = newIdent;
}

std::string User::host() {
	return userHost;
}

void User::host(std::string newHost) {
	userHost = newHost;
}

std::string User::gecos() {
	return GECOS;
}

void User::gecos(std::string newGecos) {
	GECOS = newGecos;
}

time_t User::nickTime() {
	return nickTime;
}

void User::updateTime(time_t time) {
	nickTime = time;
}

std::string User::opertype() {
	return oper;
}

void User::operup(std::string opertype) {
	oper = opertype;
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
	std::set<std::string>::iterator chanIter = inChannels.find(channel);
	if (chanIter != inChannels.end())
		inChannels.erase(chanIter);
}

std::set<std::string> User::statuses(std::string channel) {
	std::tr1::unordered_map<std::string, std::set<std::string> >::iterator chanIter = inChannels.find(channel);
	if (chanIter == inChannels.end())
		return std::set<std::string> ();
	return chanIter->second;
}

void User::addStatus(std::string channel, std::string status) {
	std::tr1::unordered_map<std::string, std::set<std::string> >::iterator chanIter = inChannels.find(channel);
	if (chanIter != inChannels.end())
		chanIter->second.insert(status);
}

void User::removeStatus(std::string channel, std::string status) {
	std::tr1::unordered_map<std::string, std::set<std::string> >::iterator chanIter = inChannels.find(channel);
	if (chanIter != inChannels.end())
		chanIter->second.erase(chanIter->second.find(status));
}

void User::changeMetadata(std::string key, std::string value) {
	metadata[key] = value;
}

std::string User::seeMetadata(std::string key) {
	return metadata[key];
}

Channel::Channel(time_t creation) : createTime(creation) {}

std::set<std::string> Channel::modes() {
	return chanModes;
}

void Channel::addMode(std::string mode, bool list) {
	if (list || mode.find_first_of('=') == std::string::npos) {
		chanModes.insert(mode);
		return;
	}
	std::string modeonly = mode.substr(0, mode.find_first_of('=')), param = mode.substr(mode.find_first_of('=') + 1);
	chanModes.insert(modeOnly);
	modeParams.insert(std::pair<std::string, std::string> (modeOnly, param));
}

void Channel::removeMode(std::string mode) {
	std::set<std::string>::iterator modeIter = chanModes.find(mode);
	if (modeIter != chanModes.end())
		chanModes.erase(modeIter);
	if (modeParams.find(mode.substr(0, mode.find_first_of('='))) != modeParams.end())
		modeParams.erase(modeParams.find(mode.substr(0, mode.find_first_of('='))));
}

std::set<std::string> Channel::users() {
	return chanUsers;
}

void Channel::joinUser(std::string user) {
	chanUsers.insert(user);
}

void Channel::partUser(std::string user) {
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

void Channel::topic(std::string newTopic, time_t tTime) {
	chanTopic = newTopic;
	topicTime = tTime;
}

time_t Channel::topicSetTime() {
	return topicTime;
}

time_t Channel::creationTime() {
	return createTime;
}

void Channel::changeMetadata(std::string key, std::string value) {
	metadata[key] = value;
}

std::string Channel::seeMetadata(std::string key) {
	return metadata[key];
}

InspIRCd::InspIRCd(std::string serverAddr, std::tr1::unordered_map<std::string, std::string> config, Base* base, unsigned short debug) : Protocol(serverAddr, config, base, debug), uidCount("AAAAAA") {
	pthread_attr_init(&detachedState);
	pthread_attr_setdetachstate(&detachedState, PTHREAD_CREATE_DETACHED);
	if (connection == NULL) {
		std::cout << "p_inspircd21: " << serverName << ": Socket handle could not be obtained." << std::endl;
		keepServer = false;
		return;
	}
	if (serverConf["bind"] != "") {
		if (!connection->bindSocket(serverConf["bind"]))
			std::cout << "Could not bind to " << serverConf["bind"] << "; trying without binding.  Abort RoBoBo and adjust configuration settings to try again with binding." << std::endl; // debug level 1
	}
}

unsigned int InspIRCd::apiVersion() {
	return 2000;
}

void InspIRCd::connectServer() {
	botBase->callPreConnectHook(serverName);
	std::istringstream portNumber (serverConf["port"]);
	unsigned short port;
	portNumber >> port;
	connection->connectServer(serverName, port);
	sleep(1);
	sendOther("CAPAB START");
	sendOther("CAPAB CAPABILITIES :PROTOCOL=1203");
	sendOther("CAPAB END");
	sendOther("SERVER " + serverConf["servername"] + " " + serverConf["password"] + " 0 " + serverConf["sid"] + " :" + serverConf["description"]);
	sendOther(":" + serverConf["sid"] + " BURST");
	sendOther(":" + serverConf["sid"] + " VERSION :RoBoBo-IRC-BoBo-2.0 InspIRCd-2.1-compat");
	unsigned int i = 0;
	std::ostringstream clientNick, clientIdent, clientHost, clientGecos, clientOper, clientChannels, currTimeS;
	clientNick << i << "/nick";
	clientIdent << i << "/ident";
	clientHost << i << "/host";
	clientGecos << i << "/gecos";
	clientOper << i << "/oper";
	clientChannels << i << "/channels";
	time_t currTime = time(NULL);
	currTimeS << currTime;
	std::tr1::unordered_map<std::string, std::string> joiningChannels;
	while (serverConf[clientNick.str()] != "") {
		std::string uuid = serverConf["sid"] + useUID();
		sendOther(":" + serverConf["sid"] + " UID " + uuid + " " + currTimeS.str() + " " + serverConf[clientNick.str()] + " " + serverConf[clientHost.str()] + " " + serverConf[clientHost.str()] + " " + serverConf[clientIdent.str()] + " 127.0.0.1 " + currTimeS.str() + " + :" + serverConf[clientGecos.str()]);
		std::tr1::unordered_map<std::string, User*>::iterator userIter = users.insert(std::pair<std::string, User*> (uuid, new User (serverConf[clientNick.str()], serverConf[clientIdent.str()], serverConf[clientHost.str()], serverConf[clientGecos.str()], currTime)));
		nicks.insert(std::pair<std::string, std::string> (serverConf[clientNick.str()], uuid));
		ourClients.insert(uuid);
		if (serverConf[clientOper.str()] != "")
			sendOther(":" + uuid + " OPERTYPE " + serverConf[clientOper.str()]);
		while (serverConf[clientChannels.str()] != "") {
			std::string channelName = serverConf[clientChannels.str()].substr(0, serverConf[clientChannels.str()].find_first_of(','));
			if (serverConf[clientChannels.str()].find_first_of(',') == std::string::npos)
				serverConf[clientChannels.str()] = "";
			else
				serverConf[clientChannels.str()] = serverConf[clientChannels.str()].substr(serverConf[clientChannels.str()].find_first_of(',') + 1);
			if (joiningChannels.find(channelName) == joiningChannels.end()) {
				joiningChannels.insert(std::pair<std::string, std::string> (channelName, "o," + uuid));
				std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = channels.insert(std::pair<std::string, Channel*> (channelName, new Channel (currTime))).first;
				chanIter->second->joinUser(uuid);
				userIter->second->joinChannel(channelName);
				userIter->second->addStatus(channelName, "op");
			} else {
				channels.find(channelName)->joinUser(uuid);
				userIter->second->joinChannel(channelName);
				userIter->second->addStatus(channelName, "op");
				joiningChannels[channelName] += " o," + uuid;
			}
		}
		i++;
		clientNick.str("");
		clientIdent.str("");
		clientHost.str("");
		clientGecos.str("");
		clientOper.str("");
		clientChannels.str("");
		clientNick << i << "/nick";
		clientIdent << i << "/ident";
		clientHost << i << "/host";
		clientGecos << i << "/gecos";
		clientOper << i << "/oper";
		clientChannels << i << "/channels";
	}
	for (std::tr1::unordered_map<std::string, std::string>::iterator jcIter = joiningChannels.begin(); jcIter != joiningChannels.end(); ++jcIter) {
		std::ostringstream currTime; // do it like this in case the second changed in the middle or something so that the correct timestamp is still sent
		currTime << channels.find(jcIter->first)->second->creationTime();
		connection->sendData(":" + serverConf["sid"] + " FJOIN " + jcIter->first + " " + currTime.str() + " +nt :" + jcIter->second);
	}
	botBase->callConnectHook(serverName);
	sendOther(":" + serverConf["sid"] + " ENDBURST");
	pthread_create(&receiveThread, &detachedState, receiveData_thread, this);
}

std::list<std::pair<std::string, char> > InspIRCd::prefixes() {
	return chanRanks;
}

std::set<char> InspIRCd::channelTypes() {
	std::set<char> chanTypes;
	chanTypes.insert('#');
	return chanTypes;
}

std::vector<std::vector<std::string> > InspIRCd::channelModes() {
	return chanModes;
}

std::list<std::string> InspIRCd::channels() {
	std::list<std::string> chanList;
	for (std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = chans.begin(); chanIter != chans.end(); ++chanIter)
		chanList.insert(chanIter->first);
	return chanList;
}

std::string InspIRCd::channelTopic(std::string channel) {
	std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = chans.find(channel);
	if (chanIter == chans.end())
		return "";
	return chanIter->second->topic();
}

std::set<std::string> InspIRCd::channelUsers(std::string channel) {
	std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = chans.find(channel);
	if (chanIter == chans.end())
		return std::set<std::string> ();
	return chanIter->second->users();
}

std::string InspIRCd::userIdent(std::string user) {
	std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(user);
	if (userIter == users.end())
		return "";
	return userIter->second->ident();
}

std::string InspIRCd::userHost(std::string user) {
	std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(user);
	if (userIter == users.end())
		return "";
	return userIter->second->host();
}

std::pair<std::string, char> InspIRCd::userStatus(std::string channel, std::string user) {
	std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(user);
	if (userIter == users.end())
		return std::pair<std::string, char> ("", ' ');
	std::set<std::string> statuses = userIter->second->statuses(channel);
	if (statuses.empty())
		return std::pair<std::string, char> ("", ' ');
	for (std::list<std::pair<std::string, char> >::iterator statIter = chanRanks.begin(); statIter != chanRanks.end(); ++statIter) {
		if (statuses.find((*statIter).first))
			return std::pair<std::string, char> ((*statIter).first, (*statIter).second);
	}
	return std::pair<std::string, char> ("", ' '); // oh noez! a bug!
}

std::string InspIRCd::compareStatus(std::set<std::string> statuses) {
	for (std::list<std::pair<std::string, char> >::iterator statIter = chanRanks.begin(); statIter != chanRanks.end(); ++statIter) {
		if (statuses.find((*statIter).first))
			return std::pair<std::string, char> ((*statIter).first, (*statIter).second);
	}
	return std::pair<std::string, char> ("", ' '); // oh noez! someone sent us a set of non-statuses!
}

void InspIRCd::sendMsg(std::string client, std::string target, std::string message) {
	if (ourClients.find(client) == ourClients.end())
		return;
	connection->sendData(":" + client + " PRIVMSG " + target + " :" + message);
}

void InspIRCd::sendNotice(std::string client, std::string target, std::string message) {
	if (ourClients.find(client) == ourClients.end())
		return;
	connection->sendData(":" + client + " NOTICE " + target + " :" + message);
}

void InspIRCd::setMode(std::string client, std::string target, std::string mode) {
	if (client != "" && ourClients.find(client) == ourClients.end())
		return;
	if (client == "")
		client = serverConf["sid"];
	std::string modes = "+", params = "";
	std::vector<std::string> modeList;
	std::string tempStr = "";
	for (size_t i = 0; i < mode.size(); i++) {
		if (mode[i] == ' ') {
			modeList.push_back(tempStr);
			tempStr++;
			continue;
		}
		tempStr += mode[i];
	}
	if (tempStr != "")
		modeList.push_back(tempStr);
	if (ourClients.find(target) != ourClients.end()) {
		std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(target);
		for (size_t i = 0; i < modeList.size(); i++)
			userIter->second->addMode(modeList[i]);
	} // Don't return because other servers should also know of the change.
	for (size_t i = 0; i < modeList.size(); i++) {
		char newMode = convertMode(modeList[i]);
		if (newMode == ' ')
			continue;
		if (modeList[i].find_first_of('=') != std::string::npos) {
			std::string newParam = modeList[i].substr(modeList[i].find_first_of('=') + 1);
			if (nicks.find(newParam) != nicks.end())
				newParam = nicks.find(newParam)->second; // Insp requires mode params that act on someone to be UUIDs so let's convert those.
			params += " " + newParam;
		}
		modes += newMode;
	}
	std::ostringstream currTime;
	currTime << time(NULL);
	connection->sendData(":" + client + " FMODE " + target + " " + currTime + " " + modes + params);
}

void InspIRCd::removeMode(std::string client, std::string target, std::string mode) {
	if (client != "" && ourClients.find(client) == ourClients.end())
		return;
	if (client == "")
		client = serverConf["sid"];
	std::string modes = "-", params = "";
	std::vector<std::string> modeList;
	std::string tempStr = "";
	for (size_t i = 0; i < mode.size(); i++) {
		if (mode[i] == ' ') {
			modeList.push_back(tempStr);
			tempStr++;
			continue;
		}
		tempStr += mode[i];
	}
	if (tempStr != "")
		modeList.push_back(tempStr);
	if (ourClients.find(target) != ourClients.end()) {
		std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(target);
		for (size_t i = 0; i < modeList.size(); i++)
			userIter->second->removeMode(modeList[i]);
	} // Don't return because other servers should also know of the change.
	for (size_t i = 0; i < modeList.size(); i++) {
		char remMode = convertMode(modeList[i]);
		if (remMode == ' ')
			continue;
		if (modeList[i].find_first_of('=') != std::string::npos) {
			std::string newParam = modeList[i].substr(modeList[i].find_first_of('=') + 1);
			if (nicks.find(newParam) != nicks.end())
				newParam = nicks.find(newParam)->second;
			params += " " + newParam;
		}
		modes += remMode;
	}
	std::ostringstream currTime;
	currTime << time(NULL);
	connection->sendData(":" + client + " FMODE " + target + " " + currTime + " " + modes + params);
}

void InspIRCd::joinChannel(std::string client, std::string channel, std::string key) {
	if (ourClients.find(client) == ourClients.end())
		return;
	std::ostringstream currTime;
	currTime << time(NULL);
	connection->sendData(":" + serverConf["sid"] + " FJOIN " + channel + " " + currTime.str() + " + :," + client);
}

void InspIRCd::partChannel(std::string client, std::string channel, std::string reason) {
	if (ourClients.find(client) == ourClients.end())
		return;
	connection->sendData(":" + client + " PART " + channel + " :" + reason);
}

void InspIRCd::quitServer(std::string reason) {
	connection->sendData(":" + serverConf["sid"] + " SQUIT " + serverConf["sid"] + " :" + reason);
	connection->closeConnection();
	keepServer = false; // if the bot is intentionally quitting, it's not necessary to keep this server anymore
}

void InspIRCd::kickUser(std::string client, std::string channel, std::string user, std::string reason) {
	if (ourClients.find(client) == ourClients.end())
		return;
	connection->sendData(":" + client + " KICK " + channel + " " + user + " :" + reason);
}

void InspIRCd::changeNick(std::string client, std::string newNick) {
	if (ourClients.find(client) == ourClients.end())
		return;
	std::ostringstream currTime;
	currTime << time(NULL);
	connection->sendData(":" + client + " NICK " + newNick + " " + currTime);
}

void InspIRCd::oper(std::string client, std::string username, std::string password) {
	if (ourClients.find(client) == ourClients.end())
		return;
	connection->sendData(":" + client + " OPERTYPE " + username);
}

void InspIRCd::killUser(std::string client, std::string user, std::string reason) {
	if (ourClients.find(client) == ourClients.end() && client != "")
		return;
	if (users.find(user) == users.end() && nicks.find(user) == nicks.end())
		return;
	if (client == "")
		client = serverConf["sid"];
	if (nicks.find(user) != nicks.end())
		user = nicks.find(user)->second;
	connection->sendData(":" + client + " KILL " + user + " :" + reason);
}

void InspIRCd::setXLine(std::string client, char lineType, std::string hostmask, time_t duration, std::string reason) {
	if (ourClients.find(client) == ourClients.end() && client != "")
		return;
	if (client == "")
		client = serverConf["sid"];
	std::ostringstream currTime, length;
	currTime << time(NULL);
	length << duration;
	connection->sendData(":" + client + " ADDLINE " + lineType + " " + hostmask + " " + client + " " + currTime.str() + " " + length.str() + " :" + reason);
}

void InspIRCd::removeXLine(std::string client, char lineType, std::string hostmask) {
	if (ourClients.find(client) == ourClients.end() && client != "")
		return;
	if (client == "")
		client = serverConf["sid"];
	connection->sendData(":" + client + " DELLINE " + lineType + " " + hostmask);
}

std::tr1::unordered_map<char, std::tr1::unordered_map<std::string, time_t> > InspIRCd::listXLines() {
	return xLines;
}

void InspIRCd::sendSNotice(char snomask, std::string text) {
	connection->sendData(":" + serverConf["sid"] + " " + snomask + " :" + text);
}

void InspIRCd::sendOther(std::string rawLine) {
	connection->sendData(rawLine);
}

void InspIRCd::addClient(std::string nick, std::string ident, std::string host, std::string gecos) {
	std::string uuid = serverConf["sid"] + useUID();
	users.insert(std::pair<std::string, User*> (uuid, new User (nick, ident, host, gecos, time(NULL))));
	nicks.insert(std::pair<std::string, std::string> (nick, uuid));
	ourClients.insert(uuid);
	std::ostringstream currTime;
	currTime << time(NULL);
	connection->sendData(":" + serverConf["sid"] + " UID " + uuid + " " + currTime.str() + " " + nick + " " + host + " " + host + " " + ident + " 127.0.0.1 " + currTime.str() + " + :" + gecos);
}

void InspIRCd::removeClient(std::string client, std::string reason) {
	if (ourClients.find(client) == ourClients.end())
		return;
	ourClients.erase(ourClients.find(client));
	std::tr1::unordered_map<std::pair, User*>::iterator userIter = users.find(client);
	nicks.erase(nicks.find(userIter->second->nick()));
	users.erase(userIter);
	connection->sendData(":" + client + " QUIT :" + reason);
}

std::set<std::string> InspIRCd::clients() {
	return ourClients;
}

std::tr1::unordered_map<std::string, std::string> InspIRCd::clientInfo(std::string client) {
	std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(client);
	if (userIter == users.end())
		return;
	std::tr1::unordered_map<std::string, std::string> info;
	info["nick"] = userIter->second->nick();
	info["ident"] = userIter->second->ident();
	info["host"] = userIter->second->host();
	info["gecos"] = userIter->second->gecos();
	std::ostringstream connTime;
	connTime << userIter->second->nickTime();
	info["connecttime"] = connTime.str();
	info["opertype"] = userIter->second->opertype();
}

std::list<std::string> InspIRCd::userModes(std::string client) {
	std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(client);
	if (userIter == users.end())
		return;
	std::list<std::string> userModes;
	std::set<std::string> umodes = userIter->second->modes();
	for (std::set<std::string>::iterator modeIter = umodes.begin(); modeIter != umodes.end(); ++modeIter)
		userModes.push_back(*modeIter);
	return userModes;
}

static void* InspIRCd::receiveData_thread(void* ptr) {
	InspIRCd* protoClass = (InspIRCd*) ptr;
	protoClass->receiveData();
	return NULL;
}

void InspIRCd::receiveData() {
	std::string receivedLine = "";
	std::vector<std::string> parsedLine;
	while (true) {
		receivedLine = connection->receive();
		if (receivedLine == "") // Some failure occured in the receivement of the line.
			break;
		parsedLine = botBase->parseLine(receivedLine);
		if (debugLevel >= 3)
			std::cout << receivedLine << std::endl;
		botBase->callPreHook(serverName, parsedLine);
		if (parsedLine[1] == "PING" && parsedLine[3] == serverConf["sid"])
			connection->sendData(":" + serverConf["sid"] + " PONG " + parsedLine[3] + parsedLine[2]);
		else if (parsedLine[0] == "CAPAB") {
			if (parsedLine[1] == "CHANMODES") {
				std::vector<std::string> splitLine;
				std::string tempStr = "";
				for (size_t i = 0; i < parsedLine[2].size(); i++) {
					if (parsedLine[2][i] == ' ') {
						splitLine.push_back(tempStr);
						tempStr = "";
						continue;
					}
					tempStr += parsedLine[2][i];
				} // line is now split by space
				if (tempStr != "")
					splitLine.push_back(tempStr); // get last fragment if line doesn't end in space
				for (size_t i = 0; i < splitLine.size(); i++) {
					std::string mode = splitLine[i].substr(0, splitLine[i].find_first_of('=')), postEqual = splitLine[i].substr(splitLine[i].find_first_of('=') + 1);
					char modeChar = postEqual[0];
					if (postEqual.size() == 2) // status mode
						modeChar = postEqual[1];
					allModes.insert(std::pair<std::string, char> (mode, modeChar));
					allChanModes.insert(std::pair<std::string, char> (modeChar, mode));
				}
			} else if (parsedLine[1] == "USERMODES") {
				std::vector<std::string> splitLine;
				std::string tempStr = "";
				for (size_t i = 0; i < parsedLine[2].size(); i++) {
					if (parsedLine[2][i] == ' ') {
						splitLine.push_back(tempStr);
						tempStr = "";
						continue;
					}
					tempStr += parsedLine[2][i];
				} // line is now split by space
				if (tempStr != "")
					splitLine.push_back(tempStr); // get last fragment if line doesn't end in space
				for (size_t i = 0; i < splitLine.size(); i++) {
					std::string mode = splitLine[i].substr(0, splitLine[i].find_first_of('=')), postEqual = splitLine[i].substr(splitLine[i].find_first_of('=') + 1);
					char modeChar = postEqual[0];
					if (postEqual.size() == 2) // status mode
						modeChar = postEqual[1];
					allModes.insert(std::pair<std::string, char> (mode, modeChar));
					allUserModes.insert(std::pair<std::string, char> (modeChar, mode));
				}
			} else if (parsedLine[1] == "CAPABILITIES") {
				std::vector<std::string> splitLine;
				std::string tempStr = "";
				for (size_t i = 0; i < parsedLine[2].size(); i++) {
					if (parsedLine[2][i] == ' ') {
						splitLine.push_back(tempStr);
						tempStr = "";
						continue;
					}
					tempStr += parsedLine[2][i];
				} // line is now split by space
				if (tempStr != "")
					splitLine.push_back(tempStr); // get last fragment if line doesn't end in space
				for (size_t i = 0; i < splitLine.size(); i++) {
					std::string first = splitLine[i].substr(0, splitLine[i].find_first_of('=')), second = splitLine[i].substr(splitLine[i].find_first_of('=') + 1);
					if (first == "PREFIX") {
						std::string third = second.substr(second.find_first_of(')') + 1);
						second = second.substr(1, second.find_first_of(')') - 1);
						for (size_t i = 0; i < second.size() && i < third.size(); i++)
							chanRanks.push_back(std::pair<std::string, char> (convertChanMode(second[i]), third[i]));
					} else if (first == "CHANMODES") {
						std::vector<std::string> modeList;
						for (size_t i = 0; i < second.size(); i++) {
							if (second[i] == ',') {
								chanModes.push_back(modeList);
								modeList.clear();
								continue;
							}
							modeList.push_back(convertChanMode(second[i]));
						}
						chanModes.push_back(modeList); // push back the last set
					}
				}
			}
		} else if (parsedLine[0] == "SERVER") {
			if (parsedLine[3] == "0")
				connectedSID = parsedLine[4];
		} else if (parsedLine[1] == "UID") {
			nicks.insert(std::pair<std::string, std::string> (parsedLine[4], parsedLine[2]));
			time_t connectTime;
			std::istringstream ct (parsedLine[9]);
			ct >> connectTime;
			std::pair<std::tr1::unordered_map<std::string, User*>::iterator, bool> newUser = users.insert(std::pair<std::string, User*> (parsedLine[2], new User (parsedLine[4], parsedLine[7], parsedLine[6], parsedLine[parsedLine.size() - 1], connectTime)));
			for (size_t i = 1; i < parsedLine[10].size(); i++) { // skip the + symbol
				std::string longmode = convertUserMode(parsedLine[10][i]);
				newUser.first->second->addMode(longmode);
				if (longmode == "snomask") {
					for (size_t j = 1; j < parsedLine[11].size(); j++)
						newUser.first->second->addSnomask(parsedLine[11][j]);
				}
			}
		} else if (parsedLine[1] == "FJOIN") {
			std::istringstream cTime (parsedLine[3]);
			time_t createTime;
			cTime >> createTime;
			std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = channels.find(parsedLine[2]);
			if (chanIter = channels.end())
				chanIter = channels.insert(std::pair<std::string, Channel*> (parsedLine[2], new Channel (createTime))).first;
			if (createTime < chanIter->second->creationTime()) {
				std::string topic = chanIter->second->topic();
				time_t topicTime = chanIter->second->topicSetTime();
				std::set<std::string> chanUsers = chanIter->second->users();
				channels.erase(chanIter);
				chanIter = channels.insert(std::pair<std::string, Channel*> (parsedLine[2], new Channel (createTime))).first;
				chanIter->second->joinUsers(chanUsers);
				for (std::set<std::string>::iterator userIter = chanUsers.begin(); userIter != chanUsers.end(); ++userIter) {
					User* user = userIter->second;
					std::set<std::string> statuses = user->statuses();
					for (std::set<std::string>::iterator prefixIter = statuses.begin(); prefixIter != statuses.end(); ++prefixIter)
						user->removeStatus(*prefixIter);
				}
				chanIter->second->topic(topic, topicTime);
				size_t param = 5; // first param
				for (size_t i = 1; i < parsedLine[4].size(); i++) {
					std::string longmode = convertChanMode(parsedLine[4][i]);
					bool foundMode = false, list = false;
					for (size_t j = 0; j < chanModes[0].size(); j++) {
						if (chanModes[0][j] == longmode)
							foundMode = list = true;
					}
					if (!foundMode) {
						for (size_t j = 0; j < chanModes[1].size(); j++) {
							if (chanModes[1][j] == longmode)
								foundMode = true;
						}
					}
					if (!foundMode) {
						for (size_t j = 0; j < chanModes[2].size(); j++) {
							if (chanModes[2][j] == longmode)
								foundMode = true;
						}
					}
					if (foundMode)
						longmode += "=" + parsedLine[param++];
					chanIter->second->addMode(longmode, list);
				}
				std::string joiningUsers = parsedLine[parsedLine.size() - 1];
				std::vector<std::string> joinUserList;
				std::string tempStr = "";
				for (size_t i = 0; i < parsedLine[2].size(); i++) {
					if (parsedLine[2][i] == ' ') {
						joinUserList.push_back(tempStr);
						tempStr = "";
						continue;
					}
					tempStr += parsedLine[2][i];
				} // line is now split by space
				if (tempStr != "")
					joinUserList.push_back(tempStr); // get last fragment if line doesn't end in space
				for (size_t i = 0; i < joinUserList.size(); i++) {
					std::list<std::string> prefixes;
					while (joinUserList[i][0] != ',') {
						prefixes.push_back(convertChanMode(joinUserList[i][0]));
						joinUserList[i] = joinUserList[i].substr(1);
					}
					chanIter->second->joinUser(joinUserList[i]);
					std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(joinUserList[i]);
					userIter->second->joinChannel(chanIter->first);
					for (std::list<std::string>::iterator prefixIter = prefixes.begin(); prefixIter != prefixes.end(); ++prefixIter)
						userIter->second->addStatus(*prefixIter);
				}
			}
		} else if (parsedLine[1] == "FMODE") {
			std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = channels.find(parsedLine[2]);
			std::istringstream cTime (parsedLine[3]);
			time_t createTime;
			cTime >> createTime;
			if (createTime <= chanIter->second->creationTime()) {
				bool adding = true;
				size_t parameter = 5;
				for (size_t i = 0; i < parsedLine[4].size(); i++) {
					if (parsedLine[4][i] == '+') {
						adding = true;
						continue;
					}
					if (parsedLine[4][i] == '-') {
						adding = false;
						continue;
					}
					std::string longmode = convertChanMode(parsedLine[4][i]);
					bool param = false, list = false;
					for (std::list<std::pair<std::string, char> >::iterator prefixIter = chanRanks.begin(); prefixIter != chanRanks.end(); ++prefixIter) {
						if (longmode == (*prefixIter).first) {
							param = true;
							if (adding)
								users.find(parsedLine[parameter++])->second->addStatus(parsedLine[2], longmode);
							else
								users.find(parsedLine[parameter++])->second->removeStatus(parsedLine[2], longmode);
							break;
						}
					}
					if (param)
						continue;
					for (size_t j = 0; j < chanModes[0].size(); j++) {
						if (chanModes[0][j] == longmode)
							param = list = true;
					}
					if (!param) {
						for (size_t j = 0; j < chanModes[1].size(); j++) {
							if (chanModes[1][j] == longmode)
								param = true;
						}
					}
					if (!param && adding) {
						for (size_t j = 0; j < chanModes[2].size(); j++) {
							if (chanModes[2][j] == longmode)
								param = true;
						}
					}
					if (param)
						longmode += "=" + parsedLine[parameter++];
					if (adding)
						chanIter->second->addMode(longmode, list);
					else
						chanIter->second->removeMode(longmode);
				}
			}
		} else if (parsedLine[1] == "MODE") {
			std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(parsedLine[0]);
			bool adding = true;
			for (size_t i = 0; i < parsedLine[2].size(); i++) {
				if (parsedLine[2][i] == '+') {
					adding = true;
					continue;
				}
				if (parsedLine[2][i] == '-') {
					adding = false;
					continue;
				}
				std::string longmode = convertUserMode(parsedLine[2][i]);
				if (longmode == "snomask") {
					bool snoAdding = true;
					for (size_t j = 0; j < parsedLine[3].size(); j++) {
						if (parsedLine[3][i] == '+') {
							snoAdding = true;
							continue;
						}
						if (parsedLine[3][i] == '-') {
							snoAdding = false;
							continue;
						}
						if (snoAdding)
							userIter->second->addSnomask(parsedLine[3][i]);
						else
							userIter->second->removeSnomask(parsedLine[3][i]);
					}
					continue;
				}
				if (adding)
					userIter->second->addMode(longmode);
				else
					userIter->second->removeMode(longmode);
			}
		} else if (parsedLine[1] == "FTOPIC") {
			std::istringstream cTime (parsedLine[3]);
			time_t topicTime;
			cTime >> topicTime;
			std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = channels.find(parsedLine[2]);
			if (topicTime > chanIter->second->topicSetTime())
				chanIter->second->topic(parsedLine[4], topicTime);
		} else if (parsedLine[1] == "FHOST")
			users.find(parsedLine[0].substr(1))->second->host(parsedLine[2]);
		else if (parsedLine[1] == "FNAME")
			users.find(parsedLine[0].substr(1))->second->gecos(parsedLine[2]);
		else if (parsedLine[1] == "NICK") {
			std::string uuid = parsedLine[0].substr(1); // strip starting colon
			std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(uuid);
			std::string oldNick = userIter->second->nick();
			userIter->second->nick(parsedLine[2]);
			nicks.erase(nicks.find(oldNick));
			nicks.insert(std::pair<std::string, std::string> (parsedLine[2], uuid));
			std::istringstream givenTimestamp (parsedLine[3]);
			time_t nickTime;
			givenTimestamp >> nickTime;
			userIter->second->updateTime(nickTime);
		} else if (parsedLine[1] == "TIME" && parsedLine[2] == serverConf["sid"] && parsedLine.size() == 4) { // don't reply if for some reason we're getting a TIME reply. That would be stupid, and you would be stupid for doing it.
			std::ostringstream currTime;
			currTime << time(NULL);
			connection->sendData(":" + serverConf["sid"] + " TIME " + parsedLine[0].substr(1) + " " + parsedLine[3] + " " + currTime.str());
		} else if (parsedLine[1] == "METADATA") {
			if (parsedLine[2][0] == '#')
				channels.find(parsedLine[2])->second->changeMetadata(parsedLine[3], parsedLine[4]);
			else
				users.find(parsedLine[2])->second->changeMetadata(parsedLine[3], parsedLine[4]);
		} else if (parsedLine[1] == "SQUIT" && (parsedLine[2] == serverConf["sid"] || parsedLine[2] == connectedSID)) {
			keepServer = false;
			connection->closeConnection();
			break;
		} else if (parsedLine[1] == "SVSJOIN" && ourClients.find(parsedLine[2]) != parsedLine.end()) {
			users.find(parsedLine[2])->second->joinChannel(parsedLine[3]);
			channels.find(parsedLine[3])->second->joinUser(parsedLine[2]);
			std::ostringstream createTime;
			createTime << channels.find(parsedLine[3])->second->creationTime();
			connection->sendData(":" + serverConf["sid"] + " FJOIN " + parsedLine[3] + " " + createTime.str() + " + ," + parsedLine[2]);
		} else if (parsedLine[1] == "SVSMODE") {
			if (parsedLine[2][0] == '#') {
				size_t param = 4;
				bool adding = true;
				for (size_t i = 0; i < parsedLine[3].size(); i++) {
					if (parsedLine[3][i] == '+') {
						adding = true;
						continue;
					}
					if (parsedLine[3][i] == '-') {
						adding = false;
						continue;
					}
					std::string longmode = convertChanMode(parsedLine[3][i]);
					bool modeParam = false;
					for (std::list<std::pair<std::string, char> >::iterator prefixIter = chanRanks.begin(); prefixIter != chanRanks.end(); ++prefixIter) {
						if (longmode == (*prefixIter).first) {
							if (nicks.find(parsedLine[param]) != nicks.end())
								parsedLine[param] = nicks.find(parsedLine[param])->second;
							if (adding)
								users.find(parsedLine[param++])->second->addStatus(parsedLine[2], longmode);
							else
								users.find(parsedLine[param++])->second->removeStatus(parsedLine[2], longmode);
							modeParam = true;
							break;
						}
					}
					if (modeParam)
						continue;
					bool list = false;
					for (size_t i = 0; i < chanModes[0].size(); i++) {
						if (longmode == chanModes[0][i])
							modeParam = list = true;
					}
					if (!modeParam) {
						for (size_t i = 0; i < chanModes[1].size(); i++) {
							if (longmode == chanModes[1][i])
								modeParam = true;
						}
					}
					if (!modeParam && adding) {
						for (size_t i = 0; i < chanModes[2].size(); i++) {
							if (longmode == chanModes[2][i])
								modeParam = true;
						}
					}
					if (modeParam)
						longmode += "=" + parsedLine[param++];
					if (adding)
						channels.find(parsedLine[2])->addMode(longmode, list);
					else
						channels.find(parsedLine[2])->removeMode(longmode, list);
				}
				std::ostringstream chanTime;
				chanTime << channels.find(parsedLine[2])->second->creationTime();
				std::string fmode = ":" + serverConf["sid"] + " FMODE " + parsedLine[2] + " " + chanTime.str() + " " + parsedLine[3];
				for (size_t i = 4; i < parsedLine.size(); i++)
					fmode += " " + parsedLine[i];
				connection->sendData(fmode);
			} else {
				if (nicks.find(parsedLine[2]) != nicks.end())
					parsedLine[2] = nicks.find(parsedLine[2])->second;
				std::tr1::unordered_map<std::string, Users*>::iterator userIter = users.find(parsedLine[2]);
				if (userIter != users.end()) {
					bool adding = true;
					for (size_t i = 0; i < parsedLine[3].size(); i++) {
						if (parsedLine[3][i] == '+') {
							adding = true;
							continue;
						}
						if (parsedLine[3][i] == '-') {
							adding = false;
							continue;
						}
						std::string longmode = convertUserMode(parsedLine[3][i]);
						if (longmode == "")
							continue;
						if (adding)
							userIter->second->addMode(longmode);
						else
							userIter->second->removeMode(longmode);
					}
					if (parsedLine.size() > 4) {
						adding = true;
						for (size_t i = 0; i < parsedLine[4].size(); i++) {
							if (parsedLine[4][i] == '+') {
								adding = true;
								continue;
							}
							if (parsedLine[4][i] == '-') {
								adding = false;
								continue;
							}
							if (adding)
								userIter->second->addSnomask(parsedLine[4][i]);
							else
								userIter->second->removeSnomask(parsedLine[4][i]);
						}
						connection->sendData(":" + serverConf["sid"] + " MODE " + userIter->first + " " + parsedLine[3] + " " + parsedLine[4]);
					} else
						connection->sendData(":" + serverConf["sid"] + " MODE " + userIter->first + " " + parsedLine[3]);
				}
			}
		} else if (parsedLine[1] == "SVSNICK" && parsedLine[2].substr(0, 3) == serverConf["sid"] && nicks.find(parsedLine[3]) == nicks.end()) { // ignore it if it's not for us or if the nick is already in use
			std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(parsedLine[2]);
			nicks.erase(nicks.find(userIter->second->nick()));
			nicks.insert(std::pair<std::string, std::string> (userIter->first, parsedLine[3]));
			userIter->second->nick(parsedLine[3]);
			std::istringstream givenTimestamp (parsedLine[4]);
			time_t nickTime;
			givenTimestamp >> nickTime;
			userIter->second->updateTime(nickTime);
			connection->sendData(":" + userIter->first + " NICK " + parsedLine[3] + " " + parsedLine[4]);
		}
		botBase->callPostHook(serverName, parsedLine);
	}
}

char InspIRCd::convertMode(std::string mode) {
	if (mode.find_first_of('=') != std::string::npos)
		mode = mode.substr(0, mode.find_first_of('='));
	std::tr1::unordered_map<std::string, char>::iterator modeIter = allModes.find(mode);
	if (modeIter == allModes.end())
		return ' ';
	return modeIter->second;
}

std::string InspIRCd::convertChanMode(char mode) {
	std::tr1::unordered_map<char, std::string>::iterator modeIter = allChanModes.find(mode);
	if (modeIter == allChanModes.end())
		return "";
	return modeIter->second;
}

std::string InspIRCd::convertUserMode(char mode) {
	std::tr1::unordered_map<char, std::string>::iterator modeIter = allUserModes.find(mode);
	if (modeIter == allUserModes.end())
		return "";
	return modeIter->second;
}

std::string InspIRCd::useUID() {
	std::string uidToUse = uidCount;
	if (uidCount[5] == 'Z') {
		uidCount[5] = 'A';
		if (uidCount[4] == 'Z') {
			uidCount[4] = 'A';
			if (uidCount[3] == 'Z') {
				uidCount[3] = 'A';
				if (uidCount[2] == 'Z') {
					uidCount[2] = 'A';
					if (uidCount[1] == 'Z') {
						uidCount[1] = 'A';
						if (uidCount[0] == 'Z')
							uidCount[0] = 'A';
						else
							uidCount[0]++;
					} else
						uidCount[1]++;
				} else
					uidCount[2]++;
			} else
				uidCount[3]++;
		} else
			uidCount[4]++;
	} else
		uidCount[5]++;
	return uidToUse;
}

extern "C" Protocol* spawn(std::string serverAddr, std::tr1::unordered_map<std::string, std::string> config, Base* base, unsigned short debugLevel) {
	return new InspIRCd (serverAddr, config, base, debugLevel);
}