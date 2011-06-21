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
		std::string hostmask();
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
		~InspIRCd();
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
		void oper(std::string client, std::string username, std::string password = "");
		void killUser(std::string client, std::string user, std::string reason);
		void setXLine(std::string client, std::string lineType, std::string hostmask, time_t duration, std::string reason);
		void removeXLine(std::string client, std::string lineType, std::string hostmask);
		std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, time_t> > listXLines();
		void sendSNotice(char snomask, std::string text);
		void sendOther(std::string rawLine);
		std::string addClient(std::string nick, std::string ident, std::string host, std::string gecos);
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
		void joinUsers(std::string channel, std::vector< std::string > userList);
		std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, time_t> > xLines;
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

std::string User::hostmask() {
	return userNick + "!" + userIdent + "@" + userHost;
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

InspIRCd::~InspIRCd() {
	pthread_cancel(receiveThread);
}

unsigned int InspIRCd::apiVersion() {
	return 2000;
}

void InspIRCd::connectServer() {
	callPreConnectHook();
	std::istringstream portNumber (serverConf["port"]);
	unsigned short port;
	portNumber >> port;
	connection->connectServer(serverName, port);
	sleep(1);
	pthread_create(&receiveThread, &detachedState, receiveData_thread, this);
	connection->sendData("CAPAB START");
	connection->sendData("CAPAB CAPABILITIES :PROTOCOL=1203");
	connection->sendData("CAPAB END");
	connection->sendData("SERVER " + serverConf["servername"] + " " + serverConf["password"] + " 0 " + serverConf["sid"] + " :" + serverConf["description"]);
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
	std::tr1::unordered_map<std::string, std::vector<std::string> > joiningChannels;
	while (serverConf[clientNick.str()] != "") {
		std::string uuid = addClient(serverConf[clientNick.str()], serverConf[clientIdent.str()], serverConf[clientHost.str()], serverConf[clientGecos.str()]);
		std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(uuid);
		if (serverConf[clientOper.str()] != "")
			oper(uuid, serverConf[clientOper.str()]);
		while (serverConf[clientChannels.str()] != "") {
			std::string channelName = serverConf[clientChannels.str()].substr(0, serverConf[clientChannels.str()].find_first_of(','));
			if (serverConf[clientChannels.str()].find_first_of(',') == std::string::npos)
				serverConf[clientChannels.str()] = "";
			else
				serverConf[clientChannels.str()] = serverConf[clientChannels.str()].substr(serverConf[clientChannels.str()].find_first_of(',') + 1);
			if (chans.find(channelName) == chans.end()) {
				userIter->second->addStatus(channelName, "op");
				joiningChannels[channelName].push_back("o," + uuid);
			} else
				joiningChannels[channelName].push_back("," + uuid);
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
		currTime << chans.find(jcIter->first)->second->creationTime();
		joinUsers(jcIter->first, jcIter->second);
	}
	for (std::set<std::string>::iterator userIter = ourClients.begin(); userIter != ourClients.end(); ++userIter)
		callConnectHook(*userIter);
	sendOther(":" + serverConf["sid"] + " ENDBURST");
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
	bool channel = false, ctcp = false;
	char status = ' ';
	if (target[0] == '#' || target[1] == '#') {
		if (target[0] != '#') {
			status = target[0];
			target = target.substr(1);
		}
		channel = true;
		if (message[0] == (char)1) {
			ctcp = true;
			message.substr(1);
			if (message[message.size() - 1] == (char)1)
				message = message.substr(0, message.size() - 1);
			message = callChannelCTCPOutHook(client, target, status, message);
			if (message == "")
				return;
			message = (char)1 + message + (char)1;
		} else {
			message = callChannelMessageOutHook(client, target, status, message);
			if (message == "")
				return;
		}
	} else {
		if (nicks.find(target) == nicks.end())
			return;
		target = nicks.find(target)->second;
		if (message[0] == (char)1) {
			ctcp = true;
			message.substr(1);
			if (message[message.size() - 1] == (char)1)
				message = message.substr(0, message.size() - 1);
			message = callUserCTCPOutHook(client, target, message);
			if (message == "")
				return;
			message = (char)1 + message + (char)1;
		} else {
			message = callUserMessageOutHook(client, target, message);
			if (message == "")
				return;
		}
	}
	if (status == ' ')
		connection->sendData(":" + client + " PRIVMSG " + target + " :" + message);
	else
		connection->sendData(":" + client + " PRIVMSG " + status + target + " :" + message);
	if (channel && ctcp)
		callChannelCTCPSendHook(client, target, status, message);
	else if (channel)
		callChannelMessageSendHook(client, target, status, message);
	else if (ctcp)
		callUserCTCPSendHook(client, target, message);
	else
		callUserMessageSendHook(client, target, message);
}

void InspIRCd::sendNotice(std::string client, std::string target, std::string message) {
	if (ourClients.find(client) == ourClients.end())
		return;
	bool channel = false, ctcp = false;
	char status = ' ';
	if (target[0] == '#' || target[1] == '#') {
		channel = true;
		if (target[0] != '#') {
			status = target[0];
			target = target.substr(1);
		}
		if (message[0] == (char)1) {
			ctcp = true;
			message = message.substr(1);
			if (message[message.size() - 1] == (char)1)
				message = message.substr(0, message.size() - 1);
			message = callChannelCTCPReplyOutHook(client, target, status, message);
			if (message == "")
				return;
			message = (char)1 + message + (char)1;
		} else {
			message = callChannelNoticeOutHook(client, target, status, message);
			if (message == "")
				return;
		}
	} else {
		if (nicks.find(target) == nicks.end())
			return;
		target = nicks.find(target)->second;
		if (message[0] == (char)1) {
			ctcp = true;
			message = message.substr(1);
			if (message[message.size() - 1] == (char)1)
				message = message.substr(0, message.size() - 1);
			message = callUserCTCPReplyOutHook(client, target, message);
			if (message == "")
				return;
			message = (char)1 + message + (char)1;
		} else {
			message = callUserNoticeOutHook(client, target, message);
			if (message == "")
				return;
		}
	}
	if (status == ' ')
		connection->sendData(":" + client + " NOTICE " + target + " :" + message);
	else
		connection->sendData(":" + client + " NOTICE " + status + target + " :" + message);
	if (channel && ctcp)
		callChannelCTCPReplySendHook(client, target, status, message);
	else if (channel)
		callChannelNoticeSendHook(client, target, status, message);
	else if (ctcp)
		callUserCTCPReplySendHook(client, target, message);
	else
		callUserNoticeSendHook(client, target, message);
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
		std::string newParam = "";
		if (modeList[i].find_first_of('=') != std::string::npos) {
			newParam = modeList[i].substr(modeList[i].find_first_of('=') + 1);
			if (nicks.find(newParam) != nicks.end())
				newParam = nicks.find(newParam)->second; // Insp requires mode params that act on someone to be UUIDs so let's convert those.
			params += " " + newParam;
		}
		modes += newMode;
		if (target[0] == '#') {
			callChannelModePreHook(target, users.find(client)->second->nick(), modeList[i].substr(0, modeList[i].find_first_of('=')), true, newParam);
			bool listmode = false;
			for (size_t j = 0; j < chanModes[0].size(); j++) {
				if (modeList[i] == chanModes[0][j]) {
					listmode = true;
					break;
				}
			}
			chans.find(target)->second->addMode(modeList[i], listmode);
			std::ostringstream currTime;
			currTime << time(NULL);
			connection->sendData(":" + client + " FMODE " + target + " " + currTime + " " + modes + params);
			callChannelModePostHook(target, users.find(client)->second->nick(), modeList[i].substr(0, modeList[i].find_first_of('=')), true, newParam);
		} else {
			if (nicks.find(target) != nicks.end())
				target = nicks.find(target)->second; // Convert the target nick to a UUID since that's kinda important and stuff.
			callUserModePreHook(target, modeList[i], true);
			users.find(target)->second->addMode(modeList[i]);
			connection->sendData(":" + client + " MODE " + target + " " + modeList[i]);
			callUserModePostHook(target, modeList[i], true);
		}
	}
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
		std::string newParam = "";
		if (modeList[i].find_first_of('=') != std::string::npos) {
			newParam = modeList[i].substr(modeList[i].find_first_of('=') + 1);
			if (nicks.find(newParam) != nicks.end())
				newParam = nicks.find(newParam)->second;
			params += " " + newParam;
		}
		modes += remMode;
		if (target[0] == '#') {
			callChannelModePreHook(target, users.find(client)->second->nick(), modeList[i].substr(0, modeList[i].find_first_of('=')), false, newParam);
			bool listmode = false;
			for (size_t j = 0; j < chanModes[0].size(); j++) {
				if (modeList[i] == chanModes[0][j]) {
					listmode = true;
					break;
				}
			}
			chans.find(target)->second->addMode(modeList[i], listmode);
			std::ostringstream currTime;
			currTime << time(NULL);
			connection->sendData(":" + client + " FMODE " + target + " " + currTime + " " + modes + params);
			callChannelModePostHook(target, users.find(client)->second->nick(), modeList[i].substr(0, modeList[i].find_first_of('=')), false, newParam);
		} else {
			if (nicks.find(target) != nicks.end())
				target = nicks.find(target)->second; // Convert the target nick to a UUID since that's kinda important and stuff.
			callUserModePreHook(target, modeList[i], false);
			users.find(target)->second->addMode(modeList[i]);
			connection->sendData(":" + client + " MODE " + target + " " + modeList[i]);
			callUserModePostHook(target, modeList[i], false);
		}
	}
}

void InspIRCd::joinChannel(std::string client, std::string channel, std::string key) {
	if (ourClients.find(client) == ourClients.end())
		return;
	std::vector<std::string> joining;
	if (chans.find(channel) == chans.end())
		joining.push_back("o," + client);
	else
		joining.push_back("," + client);
	joinUsers(channel, joining);
}

void InspIRCd::partChannel(std::string client, std::string channel, std::string reason) {
	if (ourClients.find(client) == ourClients.end())
		return;
	std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(client);
	callChannelPartPreHook(channel, userIter->second->hostmask(), reason);
	userIter->second->partChannel(channel);
	chans.find(channel)->second->partUser(client);
	connection->sendData(":" + client + " PART " + channel + " :" + reason);
	callChannelPartPostHook(channel, userIter->second->hostmask(), reason);
}

void InspIRCd::quitServer(std::string reason) {
	for (std::set<std::string>::iterator userIter = ourClients.begin(); userIter != ourClients.end(); ++userIter)
		callQuitHook(*userIter);
	connection->sendData(":" + serverConf["sid"] + " SQUIT " + serverConf["sid"] + " :" + reason);
	connection->closeConnection();
	keepServer = false; // if the bot is intentionally quitting, it's not necessary to keep this server anymore
	// The bot automatically detects where the keepServer variable is when the server check comes around and uses it to decide whether to restart the server.
}

void InspIRCd::kickUser(std::string client, std::string channel, std::string user, std::string reason) {
	if (ourClients.find(client) == ourClients.end())
		return;
	if (nicks.find(user) == nicks.end())
		return;
	std::string target = nicks.find(user)->second;
	callChannelKickPreHook(channel, users.find(client)->second->nick(), user, reason);
	chans.find(channel)->second->partUser(target);
	users.find(target)->second->partChannel(channel);
	connection->sendData(":" + client + " KICK " + channel + " " + user + " :" + reason);
	callChannelKickPostHook(channel, users.find(client)->second->nick(), user, reason);
}

void InspIRCd::changeNick(std::string client, std::string newNick) {
	if (ourClients.find(client) == ourClients.end())
		return;
	std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(client);
	std::string oldNick = userIter->second->nick();
	callNickChangePreHook(oldNick, newNick);
	time_t nickTime = time(NULL);
	std::ostringstream currTime;
	currTime << nickTime;
	connection->sendData(":" + client + " NICK " + newNick + " " + currTime);
	userIter->second->nick(newNick);
	userIter->second->updateTime(nickTime);
	callNickChangePostHook(oldNick, newNick);
}

void InspIRCd::oper(std::string client, std::string username, std::string password) {
	if (ourClients.find(client) == ourClients.end())
		return;
	std::string nick = users.find(client)->second->nick();
	callUserOperPreHook(nick, username);
	connection->sendData(":" + client + " OPERTYPE :" + username);
	users.find(client)->second->operup(username);
	callUserOperPostHook(nick, username);
}

void InspIRCd::killUser(std::string client, std::string user, std::string reason) {
	if (ourClients.find(client) == ourClients.end() && client != "")
		return;
	std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(user);
	if (userIter == users.end() && nicks.find(user) == nicks.end())
		return;
	if (client == "")
		client = serverConf["sid"];
	if (nicks.find(user) != nicks.end()) {
		user = nicks.find(user)->second;
		userIter = users.find(user);
	}
	bool ourClient = false;
	if (user.substr(0, 3) == serverConf["sid"])
		ourClient = true;
	std::string clientNick = users.find(client)->second->nick();
	if (ourClient)
		callQuitHook(user);
	else
		callUserQuitPreHook(userIter->second->hostmask(), "Killed by " + clientNick + " (" + reason + ")");
	connection->sendData(":" + client + " KILL " + user + " :" + reason);
	std::set<std::string> userChannels = userIter->second->channels();
	for (std::set<std::string>::iterator chanIter = userChannels.begin(); chanIter != userChannels.end(); ++chanIter)
		chans.find(*chanIter)->second->partUser(user);
	nicks.erase(nicks.find(userIter->second->nick()));
	delete userIter->second;
	users.erase(userIter);
	if (ourClient)
		ourClients.erase(user);
	else
		callUserQuitPostHook(userIter->second->hostmask(), "Killed by " + clientNick + " (" + reason + ")");
}

void InspIRCd::setXLine(std::string client, std::string lineType, std::string hostmask, time_t duration, std::string reason) {
	if (ourClients.find(client) == ourClients.end() && client != "")
		return;
	if (client == "")
		client = serverConf["sid"];
	std::ostringstream currTime, length;
	currTime << time(NULL);
	length << duration;
	std::string sendLine = ":" + client + " ADDLINE " + lineType + " " + hostmask + " " + client + " " + currTime.str() + " " + length.str() + " :" + reason;
	connection->sendData(sendLine);
	std::vector<std::string> parsedLine = parseLine(sendLine);
	for (std::set<std::string>::iterator userIter = ourClients.begin(); userIter != ourClients.end(); ++userIter)
		callOtherDataHook(*userIter, parsedLine);
}

void InspIRCd::removeXLine(std::string client, std::string lineType, std::string hostmask) {
	if (ourClients.find(client) == ourClients.end() && client != "")
		return;
	if (client == "")
		client = serverConf["sid"];
	std::string sendLine = ":" + client + " DELLINE " + lineType + " " + hostmask;
	connection->sendData(sendLine);
	std::vector<std::string> parsedLine = parseLine(sendLine);
	for (std::set<std::string>::iterator userIter = ourClients.begin(); userIter != ourClients.end(); ++userIter)
		callOtherDataHook(*userIter, parsedLine);
}

std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, time_t> > InspIRCd::listXLines() {
	return xLines;
}

void InspIRCd::sendSNotice(char snomask, std::string text) {
	std::string sendLine = ":" + serverConf["sid"] + " SNONOTICE " + snomask + " :" + text;
	connection->sendData(sendLine);
	std::vector<std::string> parsedLine = parseLine(sendLine);
	for (std::set<std::string>::iterator userIter = ourClients.begin(); userIter != ourClients.end(); ++userIter)
		callOtherDataHook(*userIter, parsedLine);
}

void InspIRCd::sendOther(std::string rawLine) {
	connection->sendData(rawLine);
	std::vector<std::string> parsedLine = parseLine(rawLine);
	for (std::set<std::string>::iterator userIter = ourClients.begin(); userIter != ourClients.end(); ++userIter)
		callOtherDataHook(*userIter, parsedLine);
}

std::string InspIRCd::addClient(std::string nick, std::string ident, std::string host, std::string gecos) {
	std::string uuid = serverConf["sid"] + useUID();
	users.insert(std::pair<std::string, User*> (uuid, new User (nick, ident, host, gecos, time(NULL))));
	nicks.insert(std::pair<std::string, std::string> (nick, uuid));
	ourClients.insert(uuid);
	std::ostringstream currTime;
	currTime << time(NULL);
	std::string sendLine = ":" + serverConf["sid"] + " UID " + uuid + " " + currTime.str() + " " + nick + " " + host + " " + host + " " + ident + " 127.0.0.1 " + currTime.str() + " + :" + gecos;
	connection->sendData(sendLine);
	std::vector<std::string> parsedLine = parseLine(sendLine);
	for (std::set<std::string>::iterator userIter = ourClients.begin(); userIter != ourClients.end(); ++userIter)
		callOtherDataHook(*userIter, parsedLine);
	return uuid;
}

void InspIRCd::removeClient(std::string client, std::string reason) {
	if (ourClients.find(client) == ourClients.end())
		return;
	ourClients.erase(ourClients.find(client));
	std::tr1::unordered_map<std::pair, User*>::iterator userIter = users.find(client);
	std::set<std::string> userChannels = userIter->second->channels();
	for (std::set<std::string>::iterator chanIter = userChannels.begin(); chanIter != userChannels.end(); ++chanIter)
		chans.find(chanIter)->second->partUser(client);
	nicks.erase(nicks.find(userIter->second->nick()));
	delete userIter->second;
	users.erase(userIter);
	connection->sendData(":" + client + " QUIT :" + reason);
	callQuitHook(client, reason);
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
		parsedLine = parseLine(receivedLine);
		if (debugLevel >= 3)
			std::cout << receivedLine << std::endl;
		if (parsedLine[1] == "PING" && parsedLine[3] == serverConf["sid"]) {
			for (std::set<std::string>::iterator userIter = ourClients.begin(); userIter != ourClients.end(); ++userIter)
				callOtherDataHook(*userIter, parsedLine);
			sendOther(":" + serverConf["sid"] + " PONG " + parsedLine[3] + parsedLine[2]);
		} else if (parsedLine[0] == "CAPAB") {
			for (std::set<std::string>::iterator userIter = ourClients.begin(); userIter != ourClients.end(); ++userIter)
				callOtherDataHook(*userIter, parsedLine);
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
			for (std::set<std::string>::iterator userIter = ourClients.begin(); userIter != ourClients.end(); ++userIter)
				callOtherDataHook(*userIter, parsedLine);
		} else if (parsedLine[1] == "UID") {
			for (std::set<std::string>::iterator userIter = ourClients.begin(); userIter != ourClients.end(); ++userIter)
				callUserConnectPreHook(parsedLine[4], parsedLine[7], parsedLine[6], parsedLine[parsedLine.size() - 1]);
			nicks.insert(std::pair<std::string, std::string> (parsedLine[4], parsedLine[2]));
			time_t connectTime;
			std::istringstream ct (parsedLine[9]);
			ct >> connectTime;
			std::pair<std::tr1::unordered_map<std::string, User*>::iterator, bool> newUser = users.insert(std::pair<std::string, User*> (parsedLine[2], new User (parsedLine[4], parsedLine[7], parsedLine[6], parsedLine[parsedLine.size() - 1], connectTime))).first;
			for (size_t i = 1; i < parsedLine[10].size(); i++) { // skip the + symbol
				std::string longmode = convertUserMode(parsedLine[10][i]);
				newUser.first->second->addMode(longmode);
				if (longmode == "snomask") {
					for (size_t j = 1; j < parsedLine[11].size(); j++)
						newUser.first->second->addSnomask(parsedLine[11][j]);
				}
			}
			for (std::set<std::string>::iterator userIter = ourClients.begin(); userIter != ourClients.end(); ++userIter)
				callUserConnectPostHook(parsedLine[4], parsedLine[7], parsedLine[6], parsedLine[parsedLine.size() - 1]);
		} else if (parsedLine[1] == "OPERTYPE") {
			std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(parsedLine[0].substr(1));
			callUserOperPreHook(userIter->second->nick(), parsedLine[2]);
			userIter->second->addMode("oper");
			userIter->second->operup(parsedLine[2]);
			for (std::set<std::string>::iterator userIter = ourClients.begin(); userIter != ourClients.end(); ++userIter)
				callUserOperPostHook(userIter->second->nick(), parsedLine[2]);
		} else if (parsedLine[1] == "FJOIN") {
			std::string joiningUsers = parsedLine[parsedLine.size() - 1];
			std::vector<std::string> joinUserList;
			std::string tempStr = "";
			for (size_t i = 0; i < joiningUsers.size(); i++) {
				if (joiningUsers[i] == ' ') {
					joinUserList.push_back(tempStr);
					tempStr = "";
					continue;
				}
				tempStr += joiningUsers[i];
			} // line is now split by space
			if (tempStr != "")
				joinUserList.push_back(tempStr); // get last fragment if line doesn't end in space
			for (size_t i = 0; i < joinUserList.size(); i++)
				callChannelJoinPreHook(parsedLine[2], users.find(joinUserList[i].substr(joinUserList[i].find_first_of(',') + 1))->second->hostmask());
			std::istringstream cTime (parsedLine[3]);
			time_t createTime;
			cTime >> createTime;
			std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = chans.find(parsedLine[2]);
			if (chanIter = chans.end())
				chanIter = chans.insert(std::pair<std::string, Channel*> (parsedLine[2], new Channel (createTime))).first;
			if (createTime < chanIter->second->creationTime()) {
				std::string topic = chanIter->second->topic();
				time_t topicTime = chanIter->second->topicSetTime();
				std::set<std::string> chanUsers = chanIter->second->users();
				chans.erase(chanIter);
				chanIter = chans.insert(std::pair<std::string, Channel*> (parsedLine[2], new Channel (createTime))).first;
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
					callChannelJoinPostHook(chanIter->first, userIter->second->hostmask());
				}
			}
		} else if (parsedLine[1] == "FMODE") {
			std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = chans.find(parsedLine[2]);
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
							callChannelModePreHook(parsedLine[2], parsedLine[0].substr(1), longmode, adding, parsedLine[parameter]);
							param = true;
							if (adding)
								users.find(parsedLine[parameter])->second->addStatus(parsedLine[2], longmode);
							else
								users.find(parsedLine[parameter])->second->removeStatus(parsedLine[2], longmode);
							callChannelModePostHook(parsedLine[2], parsedLine[0].substr(1), longmode, adding, parsedLine[parameter++]);
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
					if (param) {
						callChannelModePreHook(parsedLine[2], parsedLine[0].substr(1), longmode, adding, parsedLine[parameter]);
						longmode += "=" + parsedLine[parameter];
					} else
						callChannelJoinPreHook(parsedLine[2], parsedLine[0].substr(1), longmode, adding);
					if (adding)
						chanIter->second->addMode(longmode, list);
					else
						chanIter->second->removeMode(longmode);
					if (param)
						callChannelModePostHook(parsedLine[2], parsedLine[0].substr(1), longmode, adding, parsedLine[parameter++]);
					else
						callChannelModePostHook(parsedLine[2], parsedLine[0].substr(1), longmode, adding);
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
				bool callHook = ourClients.find(userIter->first) != ourClients.end();
				if (callHook)
					callUserModePreHook(userIter->first, longmode, adding);
				if (adding)
					userIter->second->addMode(longmode);
				else
					userIter->second->removeMode(longmode);
				if (callHook)
					callUserModePostHook(userIter->first, longmode, adding);
			}
		} else if (parsedLine[1] == "FTOPIC") {
			std::istringstream cTime (parsedLine[3]);
			time_t topicTime;
			cTime >> topicTime;
			std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = chans.find(parsedLine[2]);
			if (topicTime > chanIter->second->topicSetTime())
				chanIter->second->topic(parsedLine[4], topicTime);
			for (std::set<std::string>::iterator userIter = ourClients.begin(); userIter != ourClients.end(); ++userIter) {
				std::set<std::string> channelsList = users.find(*userIter)->second->channels();
				if (channelsList.find(parsedLine[2]) != chans.end())
					callOtherDataHook(*userIter, parsedLine);
			}
		} else if (parsedLine[1] == "FIDENT") {
			users.find(parsedLine[0].substr(1))->second->ident(parsedLine[2]);
			if (ourClients.find(parsedLine[0].substr(1)) != ourClients.end())
				callOtherDataHook(parsedLine[0].substr(1), parsedLine);
		} else if (parsedLine[1] == "FHOST") {
			users.find(parsedLine[0].substr(1))->second->host(parsedLine[2]);
			if (ourClients.find(parsedLine[0].substr(1)) != ourClients.end())
				callOtherDataHook(parsedLine[0].substr(1), parsedLine);
		} else if (parsedLine[1] == "FNAME") {
			users.find(parsedLine[0].substr(1))->second->gecos(parsedLine[2]);
			if (ourClients.find(parsedLine[0].substr(1)) != ourClients.end())
				callOtherDataHook(parsedLine[0].substr(1), parsedLine);
		} else if (parsedLine[1] == "NICK") {
			std::string uuid = parsedLine[0].substr(1); // strip starting colon
			std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(uuid);
			std::string oldNick = userIter->second->nick();
			callNickChangePreHook(oldNick, parsedLine[2]);
			userIter->second->nick(parsedLine[2]);
			nicks.erase(nicks.find(oldNick));
			nicks.insert(std::pair<std::string, std::string> (parsedLine[2], uuid));
			std::istringstream givenTimestamp (parsedLine[3]);
			time_t nickTime;
			givenTimestamp >> nickTime;
			userIter->second->updateTime(nickTime);
			callNickChangePostHook(oldNick, parsedLine[2]);
		} else if (parsedLine[1] == "PART") {
			std::string uuid = parsedLine[0].substr(1); // strip starting colon
			std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(uuid);
			std::string hostmask = userIter->second->hostmask();
			callChannelPartPreHook(parsedLine[2], hostmask, parsedLine[3]);
			chans.find(parsedLine[2])->second->partUser(uuid);
			userIter->second->partChannel(parsedLine[2]);
			callChannelPartPostHook(parsedLine[2], hostmask, parsedLine[3]);
		} else if (parsedLine[1] == "QUIT") { // QUIT not valid from remote servers for local clients, so no need to check for that.
			std::string uuid = parsedLine[0].substr(1); // strip starting colon
			std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(uuid);
			std::string hostmask = userIter->second->hostmask();
			callUserQuitPreHook(hostmask, parsedLine[2]);
			std::set<std::string> userChannels = userIter->second->channels();
			for (std::set<std::string> chanIter = userChannels.begin(); chanIter != userChannels.end(); ++chanIter)
				channels.find(*chanIter)->second->partUser(uuid);
			nicks.erase(nicks.find(userIter->second->nick()));
			delete userIter->second;
			users.erase(userIter);
			callUserQuitPostHook(hostmask, parsedLine[2]);
		} else if (parsedLine[1] == "TIME" && parsedLine[2] == serverConf["sid"] && parsedLine.size() == 4) { // don't reply if for some reason we're getting a TIME reply. That would be stupid, and you would be stupid for doing it.
			std::ostringstream currTime;
			currTime << time(NULL);
			for (std::set<std::string>::iterator userIter = ourClients.begin(); userIter != ourClients.end(); ++userIter)
				callOtherDataHook(*userIter, parsedLine);
			sendOther(":" + serverConf["sid"] + " TIME " + parsedLine[0].substr(1) + " " + parsedLine[3] + " " + currTime.str());
		} else if (parsedLine[1] == "METADATA") {
			if (parsedLine[2][0] == '#')
				chans.find(parsedLine[2])->second->changeMetadata(parsedLine[3], parsedLine[4]);
			else
				users.find(parsedLine[2])->second->changeMetadata(parsedLine[3], parsedLine[4]);
			for (std::set<std::string>::iterator userIter = ourClients.begin(); userIter != ourClients.end(); ++userIter)
				callOtherDataHook(*userIter, parsedLine);
		} else if (parsedLine[1] == "SQUIT" && (parsedLine[2] == serverConf["sid"] || parsedLine[2] == connectedSID)) {
			keepServer = false;
			connection->closeConnection();
			for (std::set<std::string>::iterator userIter = ourClients.begin(); userIter != ourClients.end(); ++userIter)
				callQuitHook(*userIter);
			break;
		} else if (parsedLine[1] == "SVSJOIN" && ourClients.find(parsedLine[2]) != parsedLine.end()) {
			std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(parsedLine[2]);
			callChannelJoinPreHook(parsedLine[3], userIter->second->hostmask());
			userIter->second->joinChannel(parsedLine[3]);
			chans.find(parsedLine[3])->second->joinUser(parsedLine[2]);
			std::ostringstream createTime;
			createTime << chans.find(parsedLine[3])->second->creationTime();
			connection->sendData(":" + serverConf["sid"] + " FJOIN " + parsedLine[3] + " " + createTime.str() + " + ," + parsedLine[2]);
			callChannelJoinPostHook(parsedLine[3], userIter->second->hostmask());
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
							callChannelModePreHook(parsedLine[2], parsedLine[0].substr(1), longmode, adding, parsedLine[param]);
							if (nicks.find(parsedLine[param]) != nicks.end())
								parsedLine[param] = nicks.find(parsedLine[param])->second;
							if (adding)
								users.find(parsedLine[param])->second->addStatus(parsedLine[2], longmode);
							else
								users.find(parsedLine[param])->second->removeStatus(parsedLine[2], longmode);
							modeParam = true;
							callChannelModePostHook(parsedLine[2], parsedLine[0].substr(1), longmode, adding, parsedLine[param++]);
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
					if (modeParam) {
						callChannelModePreHook(parsedLine[2], parsedLine[0].substr(1), longmode, adding, parsedLine[param]);
						longmode += "=" + parsedLine[param];
					} else
						callChannelModePreHook(parsedLine[2], parsedLine[0].substr(1), longmode, adding);
					if (adding)
						chans.find(parsedLine[2])->second->addMode(longmode, list);
					else
						chans.find(parsedLine[2])->second->removeMode(longmode, list);
					if (modeParam)
						callChannelModePostHook(parsedLine[2], parsedLine[0].substr(1), longmode, adding, parsedLine[param++]);
					else
						callChannelModePostHook(parsedLine[2], parsedLine[0].substr(1), longmode, adding);
				}
				std::ostringstream chanTime;
				chanTime << chans.find(parsedLine[2])->second->creationTime();
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
						if (ourClients.find(userIter->first) != ourClients.end())
							callUserModePreHook(userIter->first, longmode, adding);
						if (adding)
							userIter->second->addMode(longmode);
						else
							userIter->second->removeMode(longmode);
						if (ourClients.find(userIter->first) != ourClients.end())
							callUserModePostHook(userIter->first, longmode, adding);
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
			std::string oldnick = userIter->second->nick();
			callNickChangePreHook(oldnick, parsedLine[3]);
			nicks.erase(nicks.find(oldnick));
			nicks.insert(std::pair<std::string, std::string> (userIter->first, parsedLine[3]));
			userIter->second->nick(parsedLine[3]);
			std::istringstream givenTimestamp (parsedLine[4]);
			time_t nickTime;
			givenTimestamp >> nickTime;
			userIter->second->updateTime(nickTime);
			connection->sendData(":" + userIter->first + " NICK " + parsedLine[3] + " " + parsedLine[4]);
			callNickChangePostHook(oldnick, parsedLine[3]);
		} else if (parsedLine[1] == "SVSPART" && parsedLine[2].substr(0, 3) == serverConf["sid"]) { // ignore if not for us
			std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(parsedLine[2]);
			std::string hostmask = userIter->second->hostmask();
			callChannelPartPreHook(parsedLine[3], hostmask, "SVSPART received");
			userIter->second->partChannel(parsedLine[3]);
			std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = chans.find(parsedLine[3]);
			chanIter->second->partUser(parsedLine[2]);
			std::set<std::string> modes = chanIter->second->modes();
			if (chanIter->second->users.empty() && modes.find("permanent") == modes.end()) {
				delete chanIter->second;
				chans.erase(chanIter);
			}
			connection->sendData(":" + parsedLine[2] + " PART " + parsedLine[3] + " :SVSPART received");
			callChannelPartPostHook(parsedLine[3], hostmask, "SVSPART received");
		} else if (parsedLine[1] == "ENCAP" && (parsedLine[2] == "*" || parsedLine[2] == serverConf["sid"])) {
			if (parsedLine[3] == "ALLTIME") {
				std::ostringstream currTime;
				currTime << time(NULL);
				sendOther(":" + serverConf["sid"] + " PUSH " + parsedLine[0].substr(1) + "::" + serverConf["servername"] + " NOTICE " + users.find(parsedLine[0].substr(1))->second->nick() + " :robobo.alchemyirc.net " + currTime.str());
			} else if (parsedLine[3] == "CHGHOST") {
				users.find(parsedLine[4])->second->host(parsedLine[5]);
				sendOther(":" + parsedLine[4] + " FHOST " + parsedLine[5]);
			} else if (parsedLine[3] == "CHGIDENT") {
				users.find(parsedLine[4])->second->ident(parsedLine[5]);
				sendOther(":" + parsedLine[4] + " FIDENT " + parsedLine[5]);
			} else if (parsedLine[3] == "CHGNAME") {
				users.find(parsedLine[4])->second->gecos(parsedLine[5]);
				sendOther(":" + parsedLine[4] + " FNAME :" + parsedLine[5]);
			} else if (parsedLine[3] == "FPART") {
				users.find(parsedLine[5])->second->partChannel(parsedLine[4]);
				std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = chans.find(parsedLine[4]);
				chanIter->second->partUser(parsedLine[5]);
				std::set<std::string> modes = chanIter->second->modes();
				if (chanIter->second->users().empty() && modes.find("permanent") == modes.end()) {
					delete chanIter->second;
					chans.erase(chanIter);
				}
				if (parsedLine.size() == 6)
					partChannel(parsedLine[5], parsedLine[4], "Removed by " + users.find(parsedLine[0].substr(1))->second->nick() + ": No reason given");
				else
					partChannel(parsedLine[5], parsedLine[4], "Removed by " + users.find(parsedLine[0].substr(1))->second->nick() + ": " + parsedLine[6]);
			} else if (parsedLine[3] == "REMOVE") {
				users.find(parsedLine[4])->second->partChannel(parsedLine[5]);
				std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = chans.find(parsedLine[5]);
				chanIter->second->partUser(parsedLine[4]);
				std::set<std::string> modes = chanIter->second->modes();
				if (chanIter->second->users().empty() && modes.find("permanent") == modes.end()) {
					delete chanIter->second;
					chans.erase(chanIter);
				}
				if (parsedLine.size() == 6)
					partChannel(parsedLine[4], parsedLine[5], "Removed by " + users.find(parsedLine[0].substr(1))->second->nick() + ": No reason given");
				else
					partChannel(parsedLine[4], parsedLine[5], "Removed by " + users.find(parsedLine[0].substr(1))->second->nick() + ": " + parsedLine[6]);
			} else if (parsedLine[3] == "SAJOIN") {
				std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = chans.find(parsedLine[5]);
				bool chanCreated = false;
				if (chanIter == chans.end()) {
					chanIter = chans.insert(std::pair<std::string, Channel*> (parsedLine[5], new Channel (time(NULL)))).first;
					chanCreated = true;
				}
				chanIter->second->joinUser(parsedLine[4]);
				std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(parsedLine[4]);
				userIter->second->joinChannel(parsedLine[5]);
				std::vector<std::string> user;
				if (chanCreated) {
					userIter->second->addStatus(parsedLine[5], "op");
					user.push_back("o," + parsedLine[4]);
				} else
					user.push_back("," + parsedLine[4]);
				joinUsers(parsedLine[5], user);
			} else if (parsedLine[3] == "SAKICK") {
				std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = chans.find(parsedLine[4]);
				chanIter->second->partUser(parsedLine[5]);
				std::set<std::string> modes = chanIter->second->modes();
				if (chanIter->second->users().empty() && modes.find("permanent") == modes.end()) {
					delete chanIter->second;
					chans.erase(chanIter);
				}
				users.find(parsedLine[5])->second->partChannel(parsedLine[4]);
				if (parsedLine.size() == 6)
					kickUser(serverConf["sid"], parsedLine[4], parsedLine[5]);
				else
					kickUser(serverConf["sid"], parsedLine[4], parsedLine[5], parsedLine[6]);
			} else if (parsedLine[3] == "SANICK") {
				std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(parsedLine[4]);
				nicks.erase(nicks.find(userIter->second->nick()));
				nicks.insert(std::pair<std::string, std::string> (parsedLine[5], parsedLine[4]));
				userIter->second->nick(parsedLine[5]);
				std::ostringstream currTime;
				currTime << time(NULL);
				changeNick(userIter->first, parsedLine[5]);
			} else if (parsedLine[3] == "SAPART") {
				std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = chans.find(parsedLine[5]);
				chanIter->second->partUser(parsedLine[4]);
				std::set<std::string> modes = chanIter->second->modes();
				if (chanIter->second->users().empty() && modes.find("permanent") == modes.end()) {
					delete chanIter->second;
					chans.erase(chanIter);
				}
				users.find(parsedLine[4])->second->partChannel(parsedLine[5]);
				if (parsedLine.size() == 6)
					partChannel(parsedLine[4], parsedLine[5]);
				else
					partChannel(parsedLine[4], parsedLine[5], parsedLine[6]);
			} else if (parsedLine[3] == "SAQUIT")
				removeClient(parsedLine[4], parsedLine[5]);
			else {
				for (std::set<std::string>::iterator userIter = users.begin(); userIter != users.end(); ++userIter)
					callOtherDataHook(*userIter, parsedLine);
			}
		}
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

void InspIRCd::joinUsers(std::string channel, std::vector<std::string> userList) {
	std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = chans.find(channel);
	std::string modes = "+";
	std::set<std::string> chanUsers;
	std::string joiningUsers;
	for (size_t i = 0; i < userList.size(); i++) {
		joiningUsers += " " + userList[i];
		std::string uuid = userList[i].substr(userList[i].find_first_of(',') + 1);
		std::string hostmask = users.find(uuid)->second->hostmask();
		callChannelJoinPreHook(channel, hostmask);
		chanUsers.insert(uuid);
		users.find(uuid)->second->joinChannel(channel);
	}
	if (chanIter == chans.end()) {
		chanIter = chans.insert(std::pair<std::string, Channel*> (channel, new Channel (time(NULL)))).first;
		modes += "nt";
	}
	time_t chanTime = chanIter->second->creationTime();
	std::ostringstream chanTimeS;
	chanTimeS << chanTime;
	chanIter->second->joinUsers(chanUsers);
	for (size_t i = 0; i < userList.size(); i++) {
		std::string uuid = userList[i].substr(userList[i].find_first_of(',') + 1);
		std::string hostmask = users.find(uuid)->second->hostmask();
		callChannelJoinPostHook(channel, hostmask);
	}
	joiningUsers = joiningUsers.substr(1); // remove opening space
	connection->sendData(":" + serverConf["sid"] + " FJOIN " + channel + " " + chanTimeS.str() + " " + modes + " :" + joiningUsers);
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