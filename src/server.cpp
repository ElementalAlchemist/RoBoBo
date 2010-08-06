#include "connection.h"
#include "modules.h"

Server::Server(std::string serverAddress, std::tr1::unordered_map<std::string, std::string> confVars, ModuleInterface* modFace, unsigned short debug) : serverName(serverAddress), debugLevel(debug), moduleData(modFace), serverConf(confVars) {
	pthread_mutex_init(&secondsmutex, NULL); // initialize mutex for use in sending threads
	pthread_attr_init(&detachedState);
	pthread_attr_setdetachstate(&detachedState, PTHREAD_CREATE_DETACHED);
	std::istringstream portNumber (serverConf["port"]);
	unsigned short port;
	portNumber >> port;
	if (serverConf["bind"] != "") {
		if (!serverConnection.bindSocket(serverConf["bind"]))
			std::cout << "Could not bind to " << serverConf["bind"] << "; trying without binding.  Abort RoBoBo and adjust configuration settings to try again with binding." << std::endl; // debug level 1
	}
	serverConnection.connectServer(serverAddress, port);
	sleep(1); // don't send right away in case of some sort of death or slowness
	if (serverConf["password"] != "")
		sendLine("PASS " + serverConf["password"]);
	sendLine("NICK " + serverConf["nick"]);
	sendLine("USER " + serverConf["ident"] + " here " + serverAddress + " :" + serverConf["gecos"]);
	pthread_create(&dataReceiveThread, &detachedState, handleData_thread, this);
	pthread_create(&dataSendThread, &detachedState, sendData_thread, this);
}

Server::~Server() {
	pthread_cancel(dataReceiveThread);
	pthread_cancel(dataSendThread);
	pthread_cancel(secondDecrementThread);
}

bool Server::stillConnected() {
	return serverConnection.isConnected();
}

void Server::sendLine(std::string line) {
	outData.push(line);
}

std::tr1::unordered_map<std::string, std::string> Server::getInfo() {
	return serverConf;
}

std::tr1::unordered_map<char, char> Server::getPrefixes() {
	return prefix;
}

std::vector<std::vector<char> > Server::getChanModes() {
	return chanModes;
}

std::vector<char> Server::getChanTypes() {
	return chanTypes;
}

void Server::resyncChannels() {
	for (std::tr1::unordered_map<std::string, Channel*>::iterator iter = inChannels.begin(); iter != inChannels.end(); ++iter)
		sendLine("NAMES " + iter->first);
}

std::list<std::string> Server::getChannels() {
	std::list<std::string> channelList;
	for (std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = inChannels.begin(); chanIter != inChannels.end(); ++chanIter)
		channelList.insert(channelList.end(), chanIter->first);
	return channelList;
}

std::string Server::getChannelTopic(std::string channel) {
	std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = inChannels.find(channel);
	if (chanIter == inChannels.end())
		return "";
	return chanIter->second->getTopic();
}

std::list<std::string> Server::getChannelUsers(std::string channel) {
	std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = inChannels.find(channel);
	if (chanIter == inChannels.end())
		return std::list<std::string> ();
	return chanIter->second->getUsers();
}

std::pair<char, char> Server::getUserStatus(std::string channel, std::string user) {
	std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = inChannels.find(channel);
	if (chanIter == inChannels.end())
		return std::pair<char, char> ('0', ' ');
	char status = chanIter->second->getStatus(user);
	return std::pair<char, char> (status, prefix[status]);
}

void* Server::handleData_thread(void* ptr) {
	Server* servptr = (Server*) ptr;
	servptr->handleData();
	return NULL;
}

void Server::handleData() {
	std::string receivedLine = "";
	std::vector<std::string> parsedLine;
	while (true) {
		receivedLine = serverConnection.receive();
		if (receivedLine == "")
			break; // this case indicates a receive error
		if (debugLevel >= 3)
			std::cout << receivedLine << std::endl;
		parsedLine = parseLine(receivedLine);
		moduleData->callHook(serverName, parsedLine); // call module hooks for the received message
		if (parsedLine[1] == "001") { // welcome to the network
			sendLine("MODE " + serverConf["nick"] + " +B"); // set bot mode
			if (serverConf["channels"] != "")
				sendLine("JOIN " + serverConf["channels"]);
			registered = true;
		} else if (parsedLine[1] == "005") // server features
			parse005(parsedLine);
		else if (parsedLine[1] == "332") { // channel topic
			std::tr1::unordered_map<std::string, Channel*>::iterator it = inChannels.find(parsedLine[3]);
			if (it != inChannels.end())
				it->second->setTopic(parsedLine[4]);
		} else if (parsedLine[1] == "353") { // NAMES reply
			std::tr1::unordered_map<std::string, Channel*>::iterator it = inChannels.find(parsedLine[4]);
			if (it != inChannels.end())
				it->second->parseNames(separateBySpace(parsedLine[5]));
		} else if (parsedLine[1] == "366") { // end of NAMES reply
			std::tr1::unordered_map<std::string, Channel*>::iterator it = inChannels.find(parsedLine[3]);
				it->second->numeric366();
		} else if (parsedLine[1] == "433" && !registered) { // nickname already in use
			if (!altChanged) {
				sendLine("NICK " + serverConf["altnick"]);
				serverConf["nick"] = serverConf["altnick"];
				altChanged = true;
			} else
				sendLine("QUIT");
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
							userModes.push_back(parsedLine[3][i]);
						else {
							for (std::vector<char>::iterator uModeIter = userModes.begin(); uModeIter != userModes.end(); ++uModeIter) {
								if (parsedLine[3][i] == *uModeIter) {
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
						std::tr1::unordered_map<char, char>::iterator prefixIter = prefix.find(parsedLine[3][i]);
						category = 0; // count it as a list mode since it's a list of users who hold a status
						if (prefixIter == prefix.end()) {
							bool found = false;
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
						
						if (category == 0 || category == 1 || (category == 2 && addMode)) {
							std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = inChannels.find(parsedLine[2]);
							if (chanIter != inChannels.end())
								chanIter->second->setMode(addMode, parsedLine[3][i], parsedLine[currParam++]);
						} else {
							std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = inChannels.find(parsedLine[2]);
							if (chanIter != inChannels.end())
								chanIter->second->setMode(addMode, parsedLine[3][i], "");
						}
					}
				}
			}
		} else if (parsedLine[1] == "NICK" && serverConf["nick"] == separateNickFromFullHostmask(parsedLine[0].substr(1))) // bot's nick changed
			serverConf["nick"] = parsedLine[2];
		else if (parsedLine[1] == "NICK") {
			for (std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = inChannels.begin(); chanIter != inChannels.end(); ++chanIter)
				chanIter->second->changeNick(separateNickFromFullHostmask(parsedLine[0].substr(1)), parsedLine[2]);
		} else if (parsedLine[1] == "JOIN" && serverConf["nick"] == separateNickFromFullHostmask(parsedLine[0].substr(1))) // bot joined a channel
			inChannels.insert(std::pair<std::string, Channel*> (parsedLine[2], new Channel (this)));
		else if (parsedLine[1] == "JOIN")
			inChannels.find(parsedLine[2])->second->joinChannel(separateNickFromFullHostmask(parsedLine[0].substr(1)));
		else if (parsedLine[1] == "PART" && serverConf["nick"] == separateNickFromFullHostmask(parsedLine[0].substr(1)))
			inChannels.erase(parsedLine[2]);
		else if (parsedLine[1] == "PART")
			inChannels.find(parsedLine[2])->second->leaveChannel(separateNickFromFullHostmask(parsedLine[0].substr(1)));
		else if (parsedLine[1] == "QUIT" && serverConf["nick"] == separateNickFromFullHostmask(parsedLine[0].substr(1))) {
			serverConnection.closeConnection();
			moduleData->removeServer(serverName); // The server is disconnected. Remove its instance.
			break;
		} else if (parsedLine[1] == "QUIT") {
			for (std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = inChannels.begin(); chanIter != inChannels.end(); ++chanIter)
				chanIter->second->leaveChannel(separateNickFromFullHostmask(parsedLine[0].substr(1)));
		} else if (parsedLine[1] == "KILL" && serverConf["nick"] == parsedLine[2]) {
			serverConnection.closeConnection();
			moduleData->removeServer(serverName);
			break;
		} else if (parsedLine[0] == "PING") // server ping
			sendLine("PONG " + parsedLine[1]);
		else if (parsedLine[0] == "ERROR") {
			if (parsedLine[1].size() > 12) {
				if (parsedLine[1].substr(0,12) == "Closing Link") {
					serverConnection.closeConnection();
					moduleData->removeServer(serverName);
					break;
				}
			}
		}
	}
}

void* Server::sendData_thread(void* ptr) {
	Server* servptr = (Server*) ptr;
	servptr->sendData();
	return NULL;
}

void Server::sendData() {
	seconds = 0;
	unsigned short secondsToAdd = 0;
	std::string sendingMessage = "";
	std::string command = "";
	pthread_create(&secondDecrementThread, &detachedState, secondDecrement_thread, this);
	while (true) {
		if (!serverConnection.isConnected())
			break; // Thread must die when server isn't connected.
		if (outData.empty()) {
			usleep(500000); // sleep for a half-second to avoid processor abuse while being ready for data to arrive
			continue; // check again for empty queue
		}
		sendingMessage = outData.front();
		outData.pop();
		command = sendingMessage.substr(0,sendingMessage.find_first_of(' '));
		
		if (command == "MODE") { // consolidate modes into one line
			secondsToAdd = 1;
			std::vector<std::string> parsedLine = parseLine(sendingMessage);
			std::string channel = parsedLine[1], modes = parsedLine[2], params = "";
			bool addingMode = (modes[0] == '-') ? false : true;
			if (parsedLine.size() > 3) // if there is a parameter, add it
				params += " " + parsedLine[3];
			if (modes[1] == 'x' && !moduleData->isChanType(channel[0], serverName))
				secondsToAdd = 6; // because setting umode +x is apparently such an expensive operation.
			
			if (!outData.empty()) {
				sendingMessage = outData.front();
				parsedLine = parseLine(sendingMessage);
				unsigned short limit = 1;
				while (!outData.empty() && parsedLine[0] == "MODE" && parsedLine[1] == channel && limit <= maxModes) {
					outData.pop(); // remove the message we are parsing
					if ((parsedLine[2][0] == '+') == addingMode) // if we're doing the same thing as the operation already occurring on the mode string
						modes += parsedLine[2][1];
					else
						modes += parsedLine[2];
					if (parsedLine[2][0] == '+')
						addingMode = true;
					else
						addingMode = false;
					if (*modes.rbegin() == 'x')
						secondsToAdd = 6;
					if (parsedLine.size() > 3)
						params += " " + parsedLine[3];
					limit++;
					if (!outData.empty()) {
						sendingMessage = outData.front();
						parsedLine = parseLine(sendingMessage);
					}
				}
			}
			sendingMessage = "MODE " + channel + " " + modes + params; // set the sendingMessage to the final compilation forming the message we are sending
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
		
		while (seconds + secondsToAdd > 10) {
			sleep(1);
		}
		serverConnection.sendData(sendingMessage);
		if (debugLevel >= 3)
			std::cout << " -> " << sendingMessage << std::endl;
		moduleData->callHookOut(serverName, parseLine(sendingMessage));
		pthread_mutex_lock(&secondsmutex);
		seconds += secondsToAdd;
		pthread_mutex_unlock(&secondsmutex);
	}
}

void* Server::secondDecrement_thread(void* ptr) {
	Server* servptr = (Server*) ptr;
	servptr->secondDecrement();
	return NULL;
}

void Server::secondDecrement() {
	while (true) {
		if (!serverConnection.isConnected())
			break; // thread must die when server isn't connected anymore
		sleep(1);
		if (seconds > 0) {
			pthread_mutex_lock(&secondsmutex);
			seconds--;
			pthread_mutex_unlock(&secondsmutex);
		}
	}
}

void Server::parse005(std::vector<std::string> parsedLine) {
	for (unsigned int i = 3; i < parsedLine.size(); i++) {
		if (parsedLine[i] == "NAMESX") // RoBoBo supports NAMESX
			sendLine("PROTOCTL NAMESX"); // NAMESX
		if (parsedLine[i].size() > 8) { // Network name
			if (parsedLine[i].substr(0,8) == "NETWORK=") // NETWORK=NameOfNetwork
				network = parsedLine[i].substr(8);
		}
		if (parsedLine[i].size() > 7) { // Channel statuses
			if (parsedLine[i].substr(0,7) == "PREFIX=") { // PREFIX=(qaohv)~&@%+
				std::string data = parsedLine[i].substr(8);
				std::string modes = data.substr(0, data.find_first_of(')'));
				std::string chars = data.substr(data.find_first_of(')') + 1);
				for (unsigned int i = 0; i < modes.size(); i++)
					prefix.insert(std::pair<char, char> (modes[i], chars[i]));
			}
		}
		if (parsedLine[i].size() > 10) { // Channel types
			if (parsedLine[i].substr(0,10) == "CHANTYPES=") { // CHANTYPES=#&
				std::string types = parsedLine[i].substr(10);
				for (unsigned int i = 0; i < types.size(); i++)
					chanTypes.push_back(types[i]);
			}
		}
		if (parsedLine[i].size() > 10) { // Channel modes
			if (parsedLine[i].substr(0,10) == "CHANMODES=") { // CHANMODES=Ibeg,k,FJLfjl,ABCGKMNOPQRSTcimnprst
				std::string modeList = parsedLine[i].substr(10);
				std::vector<char> currSet;
				unsigned int i;
				for (i = 0; i < modeList.size(); i++) { // list modes
					if (modeList[i] == ',')
						break;
					currSet.push_back(modeList[i]);
				}
				chanModes.push_back(currSet);
				currSet.clear();
				for (i++; i < modeList.size(); i++) { // param for set and unset; increment i off comma before continuing
					if (modeList[i] == ',')
						break;
					currSet.push_back(modeList[i]);
				}
				chanModes.push_back(currSet);
				currSet.clear();
				for (i++; i < modeList.size(); i++) { // param for set; increment i off comma
					if (modeList[i] == ',')
						break;
					currSet.push_back(modeList[i]);
				}
				chanModes.push_back(currSet);
				currSet.clear();
				for (i++; i < modeList.size(); i++) // no param; increment i off comma
					currSet.push_back(modeList[i]); // no more commas
				chanModes.push_back(currSet);
			}
		}
		if (parsedLine[i].size() > 6) { // max modes per line
			if (parsedLine[i].substr(0,6) == "MODES=") { // MODES=19
				std::istringstream maxModesInLine (parsedLine[i].substr(6));
				maxModesInLine >> maxModes;
			}
		}
	}
}

std::vector<std::string> Server::parseLine(std::string unformattedLine) {
	std::vector<std::string> parsedLine;
	std::string linePart = "";
	for (unsigned int i = 0; i < unformattedLine.size(); i++) {
		if (unformattedLine[i] == ' ') {
			parsedLine.push_back(linePart);
			linePart = "";
			continue;
		}
		if (i > 0) {
			if (unformattedLine[i] == ':' && unformattedLine[i-1] == ' ') {
				for (i++; i < unformattedLine.size(); i++)
					linePart += unformattedLine[i];
				parsedLine.push_back(linePart);
				linePart = "";
				break;
			}
		}
		linePart += unformattedLine[i];
	}
	if (linePart != "")
		parsedLine.push_back(linePart);
	return parsedLine;
}

std::vector<std::string> Server::separateBySpace(std::string joinedLine) {
	std::vector<std::string> words;
	std::string wordPart;
	for (unsigned int i = 0; i < joinedLine.size(); i++) {
		if (joinedLine[i] == ' ') {
			words.push_back(wordPart);
			wordPart = "";
			continue;
		}
		wordPart += joinedLine[i];
	}
	if (wordPart != "")
		words.push_back(wordPart);
	return words;
}

std::string Server::separateNickFromFullHostmask(std::string hostmask) {
	return hostmask.substr(0, hostmask.find_first_of('!'));
}