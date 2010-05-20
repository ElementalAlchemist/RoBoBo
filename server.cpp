#include "connection.h"
#include "modules.h"
//#include "channel.cpp"

Server::Server(std::string serverAddress, std::tr1::unordered_map<std::string, std::string> confVars, ModuleInterface* modFace) {
	pthread_mutex_init(&secondsmutex, NULL); // initialize mutex for use in sending threads
	serverName = serverAddress;
	serverConf = confVars;
	moduleData = modFace;
	std::istringstream portNumber (serverConf["port"]);
	unsigned short port;
	portNumber >> port;
	serverConnection.connectServer(serverAddress, port);
	sendLine("NICK " + serverConf["nick"]);
	sendLine("USER " + serverConf["ident"] + " here " + serverAddress + " :" + serverConf["gecos"]);
	if (serverConf["password"] != "")
		sendLine("PASS " + serverConf["password"]);
	pthread_create(&dataReceiveThread, NULL, handleData_thread, this);
	pthread_create(&dataSendThread, NULL, sendData_thread, this);
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
	for (std::tr1::unordered_map<std::string, Channel*>::iterator iter = inChannels.begin(); iter != inChannels.end(); iter++)
		sendLine("NAMES " + iter->first);
}

std::list<std::string> Server::getChannels() {
	std::list<std::string> channelList;
	for (std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = inChannels.begin(); chanIter != inChannels.end(); ++chanIter)
		channelList.insert(channelList.end(), chanIter->first);
	return channelList;
}

std::string Server::getChannelTopic(std::string channel) {
	for (std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = inChannels.begin(); chanIter != inChannels.end(); ++chanIter) {
		if (chanIter->first == channel)
			return chanIter->second->getTopic();
	}
	return "";
}

std::list<std::string> Server::getChannelUsers(std::string channel) {
	for (std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = inChannels.begin(); chanIter != inChannels.end(); ++chanIter) {
		if (chanIter->first == channel)
			return chanIter->second->getUsers();
	}
	return std::list<std::string> (); // Return a blank list for a nonexistant channel.
}

std::pair<char, char> Server::getUserStatus(std::string channel, std::string user) {
	char status = '0';
	for (std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = inChannels.begin(); chanIter != inChannels.end(); ++chanIter) {
		if (channel == chanIter->first)
			status = chanIter->second->getStatus(user);
	}
	if (status == '0')
		return std::pair<char, char> ('0', ' ');
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
		std::cout << receivedLine << std::endl;
		parsedLine = parseLine(receivedLine);
		if (parsedLine.size() >= 1)
			moduleData->callHook(serverName, parsedLine); // call module hooks for the received message
		if (parsedLine[1] == "001") { // welcome to the network
			sendLine("MODE " + serverConf["nick"] + " +B"); // set bot mode
			if (serverConf["channels"] != "")
				sendLine("JOIN " + serverConf["channels"]);
			registered = true;
		} else if (parsedLine[1] == "005") // server features
			parse005(parsedLine);
		else if (parsedLine[1] == "332") { // channel topic
			for (std::tr1::unordered_map<std::string, Channel*>::iterator it = inChannels.begin(); it != inChannels.end(); it++) {
				if (it->first == parsedLine[3])
					it->second->setTopic(parsedLine[4]);
			}
		} else if (parsedLine[1] == "353") { // NAMES reply
			for (std::tr1::unordered_map<std::string, Channel*>::iterator it = inChannels.begin(); it != inChannels.end(); it++) {
				if (it->first == parsedLine[4])
					it->second->parseNames(separateBySpace(parsedLine[5]));
			}
		} else if (parsedLine[1] == "366") { // end of NAMES reply
			for (std::tr1::unordered_map<std::string, Channel*>::iterator it = inChannels.begin(); it != inChannels.end(); it++) {
				if (it->first == parsedLine[3])
					it->second->numeric366();
			}
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
							for (std::vector<char>::iterator uModeIter = userModes.begin(); uModeIter != userModes.end(); uModeIter++) {
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
						bool found = false;
						int category;
						for (std::tr1::unordered_map<char, char>::iterator prefixIter = prefix.begin(); prefixIter != prefix.end(); prefixIter++) {
							if (parsedLine[3][i] == prefixIter->first) {
								found = true;
								category = 0; // count it as a list mode since it's a list of users who hold a status
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
						}
						if (!found)
							category = 4;
						
						if (category == 0 || category == 1 || (category == 2 && addMode)) {
							for (std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = inChannels.begin(); chanIter != inChannels.end(); chanIter++) {
								if (chanIter->first == parsedLine[2])
									chanIter->second->setMode(addMode, parsedLine[3][i], parsedLine[currParam++]);
							}
						} else {
							for (std::tr1::unordered_map<std::string, Channel*>::iterator chanIter = inChannels.begin(); chanIter != inChannels.end(); chanIter++) {
								if (chanIter->first == parsedLine[2])
									chanIter->second->setMode(addMode, parsedLine[3][i], "");
							}
						}
					}
				}
			}
		} else if (parsedLine[1] == "NICK" && serverConf["nick"] == separateNickFromFullHostmask(parsedLine[0].substr(1))) // bot's nick changed
			serverConf["nick"] = parsedLine[2];
		else if (parsedLine[1] == "JOIN" && serverConf["nick"] == separateNickFromFullHostmask(parsedLine[0].substr(1))) // bot joined a channel
			inChannels.insert(std::pair<std::string, Channel*> (parsedLine[2], new Channel (this)));
		else if (parsedLine[1] == "PART" && serverConf["nick"] == separateNickFromFullHostmask(parsedLine[0].substr(1)))
			inChannels.erase(parsedLine[2]);
		else if (parsedLine[1] == "QUIT" && serverConf["nick"] == separateNickFromFullHostmask(parsedLine[0].substr(1))) {
			serverConnection.closeConnection();
			moduleData->removeServer(serverName); // The server is disconnected. Remove its instance.
			break;
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
	pthread_create(&secondDecrementThread, NULL, secondDecrement_thread, this);
	timespec halfSecond;
	halfSecond.tv_sec = 0;
	halfSecond.tv_nsec = 500000000; // 500 million nanoseconds = 0.5 seconds.
	while (true) {
		if (!serverConnection.isConnected())
			break; // Thread must die when server isn't connected.
		if (outData.empty()) {
			nanosleep(&halfSecond, NULL); // sleep for a half-second to avoid processor abuse while being ready for data to arrive
			continue; // check again for empty queue
		}
		sendingMessage = outData.front();
		outData.pop();
		command = sendingMessage.substr(0,sendingMessage.find_first_of(' '));
		
		if (command == "MODE") { // consolidate modes into one line
			std::vector<std::string> parsedLine = parseLine(sendingMessage);
			std::string channel = parsedLine[1], modes = parsedLine[2], params = "";
			bool addingMode = (modes[0] == '+') ? true : false;
			if (parsedLine.size() > 3) // if there is a parameter, add it
				params += " " + parsedLine[3];
			
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
		}
		
		if (command == "GLINE" || command == "KLINE" || command == "PASS" || command == "PING" || command == "PONG" || command == "QLINE" || command == "USER" || command == "ZLINE" || command == "OJOIN" || command == "SAJOIN" || command == "SAKICK" || command == "SAMODE" || command == "SANICK" || command == "SAPART" || command == "SAQUIT" || command == "SATOPIC")
			secondsToAdd = 0;  // add the correct number of seconds for the command being sent
		else if (command == "JOIN" || command == "MAP" || command == "REHASH" || command == "TOPIC" || command == "WHO" || command == "WHOIS" || command == "WHOWAS")
			secondsToAdd = 2;
		else if (command == "NICK" || command == "CYCLE")
			secondsToAdd = 3;
		else if (command == "INVITE")
			secondsToAdd = 4;
		else if (command == "LIST" || command == "PART" || command == "MKPASSWD")
			secondsToAdd = 5;
		else // all commands not on the list are worth 1 second.
			secondsToAdd = 1;
		
		while (seconds + secondsToAdd > 10) {
			sleep(1);
		}
		serverConnection.sendData(sendingMessage);
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