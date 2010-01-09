#include "connection.h"
#include "channel.cpp"

#ifndef SERVER_CPP
#define SERVER_CPP
Server::Server(std::string serverAddress, std::tr1::unordered_map<std::string, std::string> confVars, ModuleInterface* modFace) {
	pthread_mutex_init(&secondsmutex, NULL); // initialize mutex for use in sending threads
	serverName = serverAddress;
	serverConf = confVars;
	moduleData = modFace;
	std::istringstream portNumber (serverConf["port"]);
	unsigned int port;
	portNumber >> port;
	serverConnection.connectServer(serverAddress, port);
	sendLine("NICK " + serverConf["nick"]);
	sendLine("USER " + serverConf["ident"] + " here " + serverAddress + " :" + serverConf["gecos"]);
	pthread_create(&dataReceiveThread, NULL, handleData_thread, this);
	pthread_create(&dataSendThread, NULL, sendData_thread, this);
}

Server::~Server() {
	pthread_exit(&dataReceiveThread);
	pthread_exit(&dataSendThread);
	pthread_exit(&secondDecrementThread);
}

/*void Server::sendLine(std::string line) {
	serverConnection.sendData(line);
	std::cout << " -> " << line << std::endl;
	moduleData->callHookOut(serverName, parseLine(line));
}*/

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
			sendLine("NICK " + serverConf["altnick"]);
			serverConf["nick"] = serverConf["altnick"];
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
							for (unsigned int j = 0; j < chanModes[0].size(); j++) {
								if (parsedLine[3][i] == chanModes[0][j]) {
									found = true;
									category = 0;
								}
							}
						}
						if (!found) {
							for (unsigned int j = 0; j < chanModes[1].size(); j++) {
								if (parsedLine[3][i] == chanModes[1][j]) {
									found = true;
									category = 1;
								}
							}
						}
						if (!found) {
							for (unsigned int j = 0; j < chanModes[2].size(); j++) {
								if (parsedLine[3][i] == chanModes[2][j]) {
									found = true;
									category = 2;
								}
							}
						}
						if (!found) {
							for (unsigned int j = 0; j < chanModes[3].size(); j++) {
								if (parsedLine[3][i] == chanModes[3][j]) {
									found = true;
									category = 3;
								}
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
			break;
		} else if (parsedLine[1] == "KILL" && serverConf["nick"] == parsedLine[2]) {
			serverConnection.closeConnection();
			break;
		} else if (parsedLine[0] == "PING") // server ping
			sendLine("PONG " + parsedLine[1]);
		else if (parsedLine[0] == "ERROR") {
			if (parsedLine[1].size() > 12) {
				if (parsedLine[1].substr(0,12) == "Closing Link") {
					serverConnection.closeConnection();
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
		if (outData.empty()) {
			std::cout << "No data is being sent." << std::endl;
			nanosleep(&halfSecond, NULL); // sleep for a half-second to avoid processor abuse while being ready for data to arrive
			continue; // start loop over
		}
		std::cout << "Sending a message.  outData.size() == " << outData.size() << "; seconds == " << seconds << ";" << std::endl;
		sendingMessage = outData.front();
		outData.pop();
		command = sendingMessage.substr(0,sendingMessage.find_first_of(' '));
		
		if (command == "ELINE" || command == "GLINE" || command == "KLINE" || command == "ZLINE" || command == "KILL" || command == "PING" || command == "PONG" || command == "USER" || command == "SAMODE" || command == "SAJOIN" || command == "SAPART" || command == "SAQUIT" || command == "SANICK" || command == "SATOPIC") // add the correct number of seconds by the command being sent
			secondsToAdd = 0;
		else if (command == "JOIN" || command == "MAP" || command == "OPER" || command == "TOPIC" || command == "WHO" || command == "WHOIS" || command == "WHOWAS")
			secondsToAdd = 2;
		else if (command == "REHASH")
			secondsToAdd = 3;
		else if (command == "INVITE")
			secondsToAdd = 4;
		else if (command == "LIST" || command == "CYCLE")
			secondsToAdd = 5;
		else
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
		sleep(1);
		if (seconds > 0) {
			pthread_mutex_lock(&secondsmutex);
			seconds--;
			std::cout << "Seconds decremented: " << seconds << std::endl;
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
#endif