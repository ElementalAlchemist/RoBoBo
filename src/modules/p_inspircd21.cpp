#include "protoinclude.h"
#include <ctime>

class InspIRCd;
class User {
	public:
		User(std::string theNick, std::string theIdent, std::string theHost, std::string theGecos, time_t theConnectTime);
		std::string nick();
		std::string ident();
		std::string host();
		std::string gecos();
		time_t connectionTime();
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
	private:
		std::string userNick, userIdent, userHost, GECOS, oper;
		time_t connectTime;
		std::set<std::string> userModes;
		std::set<char> SNOMasks;
		std::tr1::unordered_map<std::string, std::set<std::string> > inChannels;
};

class Channel {
	public:
		Channel(time_t creation);
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
		time_t createTime;
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
		void oper(std::string client, std::string username, std::string password, std::string opertype);
		void killUser(std::string client, std::string user, std::string reason);
		void setXLine(std::string client, char lineType, std::string hostmask, time_t duration, std::string reason);
		void removeXLine(std::string client, char lineType, std::string hostmask);
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
		std::set<std::string> ourClients;
		std::tr1::unordered_map<std::string, User*> users;
		std::tr1::unordered_map<std::string, std::string> nicks;
		std::tr1::unordered_map<std::string, Channel*> chans;
		std::list<std::pair<std::string, char> > chanRanks;
		std::vector<std::vector<std::string> > chanModes;
		char convertMode(std::string mode);
		std::string uidCount;
		std::string useUID();
};

User::User(std::string theNick, std::string theIdent, std::string theHost, std::string theGecos, time_t theConnectTime) : userNick(theNick), userIdent(theIdent), userHost(theHost), GECOS(theGecos), connectTime(theConnectTime) {}

std::string User::nick() {
	return userNick;
}

std::string User::ident() {
	return userIdent;
}

std::string User::host() {
	return userHost;
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

Channel::Channel(time_t creation) : createTime(creation) {}

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

void Channel::topic(std::string newTopic) {
	chanTopic = newTopic;
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
	std::ostringstream clientNick, clientIdent, clientHost, clientGecos, clientOper, currTimeS;
	clientNick << i << "/nick";
	clientIdent << i << "/ident";
	clientHost << i << "/host";
	clientGecos << i << "/gecos";
	clientOper << i << "/oper";
	time_t currTime = time(NULL);
	currTimeS << currTime;
	while (serverConf[clientNick.str()] != "") {
		std::string uuid = serverConf["sid"] + useUID();
		sendOther(":" + serverConf["sid"] + " UID " + uuid + " " + currTimeS.str() + " " + serverConf[clientNick.str()] + " " + serverConf[clientHost.str()] + " " + serverConf[clientHost.str()] + " " + serverConf[clientIdent.str()] + " 127.0.0.1 " + currTimeS.str() + " + :" + serverConf[clientGecos.str()]);
		users.insert(std::pair<std::string, User*> (uuid, new User (serverConf[clientNick.str()], serverConf[clientIdent.str()], serverConf[clientHost.str()], serverConf[clientGecos.str()], currTime)));
		nicks.insert(std::pair<std::string, std::string> (serverConf[clientNick.str()], uuid));
		ourClients.insert(uuid);
		if (serverConf[clientOper.str()] != "")
			sendOther (":" + uuid + " OPERTYPE " + serverConf[clientOper.str()]);
		i++;
		clientNick.str("");
		clientIdent.str("");
		clientHost.str("");
		clientGecos.str("");
		clientOper.str("");
		clientNick << i << "/nick";
		clientIdent << i << "/ident";
		clientHost << i << "/host";
		clientGecos << i << "/gecos";
		clientOper << i << "/oper";
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
	for (size_t i = 0; i < modeList.size(); i++) {
		if (modeList[i].find_first_of('=') != std::string::npos) {
			std::string newParam = modeList[i].substr(modeList[i].find_first_of('=') + 1);
			if (nicks.find(newParam) != nicks.end())
				newParam = nicks.find(newParam)->second; // Insp requires mode params that act on someone to be UUIDs so let's convert those.
			params += " " + newParam;
		}
		modes += convertMode(modeList[i]);
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
	for (size_t i = 0; i < modeList.size(); i++) {
		if (modeList[i].find_first_of('=') != std::string::npos) {
			std::string newParam = modeList[i].substr(modeList[i].find_first_of('=') + 1);
			if (nicks.find(newParam) != nicks.end())
				newParam = nicks.find(newParam)->second;
			params += " " + newParam;
		}
		modes += convertMode(modeList[i]);
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

void InspIRCd::oper(std::string client, std::string username, std::string password, std::string opertype) {
	if (ourClients.find(client) == ourClients.end())
		return;
	connection->sendData(":" + client + " OPERTYPE " + opertype);
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