#include "protocol.h"
#include <atomic>
#include <deque>
#include <tuple>
#include <utility>

class User {
	public:
		User(std::string theNick, std::string theIdent, std::string theHost);
		std::string nick;
		std::string ident;
		std::string host;
		std::set<std::string> channels;
};

class Channel {
	public:
		Channel();
		std::string topic;
		std::string topicSetter;
		std::list<std::string> modes;
		std::unordered_map<std::string, std::list<std::string>> listModes;
		std::unordered_map<std::string, bool> listComplete;
		std::set<std::string> users;
		std::unordered_map<std::string, std::list<char>> statuses;
		time_t timestamp;
		bool namesComplete;
		bool whoCalled;
};

class LocalClient : public User {
	public:
		LocalClient(std::string clientid, std::string theNick, std::string theIdent, std::string theHost, std::string theGecos, Client* modClass);
		std::string id;
		std::string altNick;
		std::string gecos;
		std::set<std::string> modes;
		std::set<char> snomasks;
		bool registered;
		bool triedAlt;
		std::shared_ptr<Socket> connection;
		std::deque<std::string> sendQueue;
		std::thread receiveThread, sendThread, secondsThread;
		void receive();
		void send();
		void decrementSeconds();
		void sendLine(const std::string& line);
		std::atomic<unsigned int> seconds;
	private:
		void processSend(const std::string& message);
		Client* module;
};

User::User(std::string theNick, std::string theIdent, std::string theHost) : nick(theNick), ident(theIdent), host(theHost) {}

Channel::Channel() : timestamp(0), namesComplete(false), whoCalled(false) {}

LocalClient::LocalClient(std::string clientid, std::string theNick, std::string theIdent, std::string theHost, std::string theGecos, Client* modClass) : User(theNick, theIdent, theHost), id(clientid), gecos(theGecos), registered(false), triedAlt(false), module(modClass) {}

void LocalClient::receive() {
	while (true) {
		if (!connection->isConnected())
			return;
		std::string line = connection->receive();
		if (line == "")
			return; // A connection error.
		module->processIncoming(id, line);
	}
}

void LocalClient::send() {
	while (true) {
		if (!connection->isConnected())
			return;
		while (sendQueue.empty()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			continue;
		}
		std::string message = sendQueue.front();
		sendQueue.pop_front();
		processSend(message);
	}
}

void LocalClient::decrementSeconds() {
	while (true) {
		if (!connection->isConnected())
			return;
		std::this_thread::sleep_for(std::chrono::seconds(1));
		if (seconds > 0)
			seconds--;
	}
}

void LocalClient::sendLine(const std::string& line) {
	if (connection->isConnected()) {
		if (module->floodControl)
			sendQueue.push_back(line);
		else
			processSend(line);
	}
}

/* TODO: fix up the reliance on the queue in this function
 * Possibly make separate line splitters: one that uses the queue, and one that just keeps a list (maybe combine them?)
 * May as well split the delay out while I'm splitting this function up anyway
 */
void LocalClient::processSend(const std::string& message) {
	std::vector<std::string> parsedLine = module->parseLine(message);
	std::string fullLine = ":" + nick + "!" + ident + "@" + host + " " + message + "\r\n";
	if (fullLine.size() > 512) {
		unsigned int currParam = 3;
		if (parsedLine[0] == "MODE") {
			std::list<std::string> modeLines;
			bool channel = (module->channelTypes.find(parsedLine[1][0]) != module->channelTypes.end());
			std::string modeStr, params;
			bool adding = true, invalid = false;
			for (char mode : parsedLine[2]) {
				if (mode == '+')
					adding = true;
				else if (mode == '-')
					adding = false;
				else {
					std::string newMode;
					if (adding) {
						if (channel)
							newMode = module->convertChanMode.find(mode)->second;
						else
							newMode = module->convertUserMode.find(mode)->second;
					} else {
						if (channel)
							newMode = module->convertChanMode.find(mode)->second;
						else
							newMode = module->convertUserMode.find(mode)->second;
					}
					bool takesParam = false;
					for (std::pair<std::string, char> status : module->prefixes) {
						if (status.first == newMode)
							takesParam = true;
					}
					if (!takesParam && (module->listModes.find(newMode) != module->listModes.end() || module->paramParamModes.find(newMode) != module->paramParamModes.end() || (adding && module->paramModes.find(newMode) != module->paramModes.end())))
						takesParam = true;
					std::string newParam;
					if (takesParam) {
						if (currParam >= parsedLine.size()) {
							invalid = true;
							break;
						}
						newParam = " " + parsedLine[currParam++];
					}
					if (std::string(":" + nick + "!" + ident + "@" + host + " MODE " + parsedLine[1] + " " + modeStr + std::string(mode) + params + param + "\r\n").size() < 512) {
						modeStr += mode;
						params += newParam;
					} else {
						modeLines.push_back("MODE " + parsedLine[1] + " " + modeStr + params);
						modeStr = (adding ? "+" : "-") + std::string(mode);
						params = newParam;
					}
				}
			}
			if (invalid)
				continue; // skip this mode line if it's invalid
			message = modeLines.front();
			modeLines.pop_front();
			for (std::string line : modeLines)
				sendQueue.push_front(line);
		} else if (parsedLine[0] == "PRIVMSG" || parsedLine[0] == "NOTICE") { // Other types of messages probably shouldn't be so split.
			std::string linePrefix = ":" + nick + "!" + ident + "@" + host + " " + parsedLine[0] + " " + parsedLine[1] + " :";
			size_t prefixLen = linePrefix.size();
			size_t trimmedSpacePos = parsedLine[2].rfind(' ', 510 - prefixLen);
			if (trimmedSpacePos == std::string::npos) {
				std::string trimmedMsg = parsedLine[2].substr(0, 510 - prefixLen);
				message = parsedLine[0] + " " + parsedLine[1] + " :" + trimmedMsg;
				std::stringstream colorPrefix;
				bool bold = false, italic = false, underline = false;
				unsigned int foregroundColor = 1, backgroundColor = 0;
				bool inColor = false, firstDigit = false, secondDigit = false, commaEncountered = false;
				for (char aChar : trimmedMsg) {
					if (inColor) {
						if (aChar == ',' && !commaEncountered) {
							commaEncountered = true;
							colorPrefix >> foregroundColor;
							colorPrefix.str("");
							firstDigit = false;
							secondDigit = false;
						} else if (aChar >= '0' && aChar <= '9' && !secondDigit) {
							colorPrefix << aChar;
							if (firstDigit)
								secondDigit = true;
							else
								firstDigit = true;
						} else {
							inColor = false;
							firstDigit = false;
							secondDigit = false;
							if (commaEncountered)
								colorPrefix >> backgroundColor;
							else
								colorPrefix >> foregroundColor;
							colorPrefix.str("");
							commaEncountered = false;
						}
					}
					if (aChar == (char)2)
						bold = !bold;
					else if (aChar == (char)29)
						italic = !italic;
					else if (aChar == (char)31)
						underline = !underline;
					else if (aChar == (char)22)
						std::swap(foregroundColor, backgroundColor);
					else if (aChar == (char)15) {
						bold = false;
						italic = false;
						underline = false;
						foregroundColor = 1;
						backgroundColor = 0;
						// Also set all of this stuff in case we encounter it in the middle of a color sequence.
						inColor = false;
						firstDigit = false;
						secondDigit = false;
						commaEncountered = false;
					} else if (aChar == (char)3) {
						backgroundColor = 0;
						foregroundColor = 1;
						inColor = true;
					}
				}
				colorPrefix.str(""); // If there's something left over somehow, let's kill it.
				if (bold)
					colorPrefix << (char)2;
				if (italic)
					colorPrefix << (char)29;
				if (underline)
					colorPrefix << (char)31;
				if (foregroundColor != 1 || backgroundColor != 0)
					colorPrefix << (char)3 << foregroundColor << "," << backgroundColor;
				sendQueue.push_front(parsedLine[0] + " " + parsedLine[1] + " :" + colorPrefix.str() + parsedLine[2].substr(510 - prefixLen));
			} else {
				std::string trimmedMsg = parsedLine[2].substr(0, trimmedSpacePos);
				message = parsedLine[0] + " " + parsedLine[1] + " :" + trimmedMsg;
				std::stringstream colorPrefix;
				bool bold = false, italic = false, underline = false;
				unsigned int foregroundColor = 1, backgroundColor = 0;
				bool inColor = false, firstDigit = false, secondDigit = false, commaEncountered = false;
				for (char aChar : trimmedMsg) {
					if (inColor) {
						if (aChar == ',' && !commaEncountered) {
							commaEncountered = true;
							colorPrefix >> foregroundColor;
							colorPrefix.str("");
							firstDigit = false;
							secondDigit = false;
						} else if (aChar >= '0' && aChar <= '9' && !secondDigit) {
							colorPrefix << aChar;
							if (firstDigit)
								secondDigit = true;
							else
								firstDigit = true;
						} else {
							inColor = false;
							firstDigit = false;
							secondDigit = false;
							if (commaEncountered)
								colorPrefix >> backgroundColor;
							else
								colorPrefix >> foregroundColor;
							colorPrefix.str("");
							commaEncountered = false;
						}
					}
					if (aChar == (char)2)
						bold = !bold;
					else if (aChar == (char)29)
						italic = !italic;
					else if (aChar == (char)31)
						underline = !underline;
					else if (aChar == (char)22)
						std::swap(foregroundColor, backgroundColor);
					else if (aChar == (char)15) {
						bold = false;
						italic = false;
						underline = false;
						foregroundColor = 1;
						backgroundColor = 0;
						// Also set all of this stuff in case we encounter it in the middle of a color sequence.
						inColor = false;
						firstDigit = false;
						secondDigit = false;
						commaEncountered = false;
					} else if (aChar == (char)3) {
						backgroundColor = 0;
						foregroundColor = 1;
						inColor = true;
					}
				}
				colorPrefix.str(""); // If there's something left over somehow, let's kill it.
				if (bold)
					colorPrefix << (char)2;
				if (italic)
					colorPrefix << (char)29;
				if (underline)
					colorPrefix << (char)31;
				if (foregroundColor != 1 || backgroundColor != 0)
					colorPrefix << (char)3 << foregroundColor << "," << backgroundColor;
				sendQueue.push_front(parsedLine[0] + " " + parsedLine[1] + " :" + colorPrefix.str() + parsedLine[2].substr(trimmedSpacePos));
			}
		}
	}
	if (module->floodControl) {
		unsigned int secondsToAdd = 1;
		if (parsedLine[0] == "GLINE" || parsedLine[0] == "KLINE" || parsedLine[0] == "OJOIN" || parsedLine[0] == "PASS" || parsedLine[0] == "PING" || parsedLine[0] == "PONG" || parsedLine[0] == "QLINE" || parsedLine[0] == "SAJOIN" || parsedLine[0] == "SAKICK" || parsedLine[0] == "SAMODE" || parsedLine[0] == "SANICK" || parsedLine[0] == "SAPART" || parsedLine[0] == "SAQUIT" || parsedLine[0] == "SATOPIC" || parsedLine[0] == "USER" || parsedLine[0] == "ZLINE")
			secondsToAdd = 0;
		else if (parsedLine[0] == "JOIN" || parsedLine[0] == "MAP" || parsedLine[0] == "REHASH" || parsedLine[0] == "TOPIC" || parsedLine[0] == "WHOIS" || parsedLine[0] == "WHOWAS")
			secondsToAdd = 2;
		else if (parsedLine[0] == "CYCLE" || parsedLine[0] == "WHO")
			secondsToAdd = 3;
		else if (parsedLine[0] == "INVITE" || parsedLine[0] == "NICK")
			secondsToAdd = 4;
		else if (parsedLine[0] == "KNOCK" || parsedLine[0] == "LIST" || parsedLine[0] == "MKPASSWD" || parsedLine[0] == "PART")
			secondsToAdd = 5;
		while (seconds + secondsToAdd > 10)
			std::this_thread::sleep_for(std::chrono::seconds(1));
		seconds += secondsToAdd;
	}
	if (parsedLine[0] == "PRIVMSG") {
		size_t ctcpCount = 0;
		for (char msgChar : parsedLine[2]) {
			if (msgChar == (char)1)
				ctcpCount++;
		}
		// Detect whether the whole message is a CTCP
		if ((ctcpCount == 1 && parsedLine[2][0] == (char)1) || (ctcpCount == 2 && parsedLine[2][0] == (char)1 && parsedLine[2][parsedLine[2].size() - 1] == (char)1)) {
			std::string ctcpMsg = parsedLine[2].substr(1);
			if (parsedLine[2][parsedLine[2].size() - 1] == (char)1)
				ctcpMsg = ctcpMsg.substr(0, ctcpMsg.size() - 1);
			size_t ctcpSpace = ctcpMsg.find(' ');
			std::string ctcp = ctcpMsg.substr(0, ctcpSpace);
			std::string params;
			if (ctcpSpace != std::string::npos)
				params = ctcpMsg.substr(ctcpSpace + 1);
			// Call the appropriate hook based on whether it's being sent to a channel or a user
			if (module->chanTypes.find(parsedLine[1][0]) != module->chanTypes.end())
				module->callChanCTCPSendHook(id, parsedLine[1], ' ', ctcp, params);
			else if (module->chanTypes.find(parsedLine[1][1]) != module->chanTypes.end()) {
				bool chanCTCP = false;
				for (std::pair<std::string, char> status : module->prefixes) {
					if (status.second == parsedLine[1][0]) {
						chanCTCP = true;
						module->callChanCTCPSendHook(id, parsedLine[1].substr(1), parsedLine[1][0], ctcp, params);
						break;
					}
				}
				if (!chanCTCP)
					module->callUserCTCPSendHook(id, parsedLine[1], ctcp, params);
			} else
				module->callUserCTCPSendHook(id, parsedLine[1], ctcp, params);
		} else {
			// Either none or part of the message is CTCP; pick out the CTCP parts to make appropriate hook calls
			std::list<std::pair<std::string, std::string>> ctcpBits;
			size_t ctcpPos = 0;
			ctcpPos = parsedLine[2].find((char)1);
			while (ctcpPos != std::string::npos) {
				size_t startCTCP = ctcpPos;
				ctcpPos = parsedLine[2].find((char)1, ctcpPos + 1);
				std::string ctcpSegment = parsedLine[2].substr(startCTCP + 1, ctcpPos - (startCTCP + 1));
				if (!ctcpSegment.empty()) {
					size_t spacePos = ctcpSegment.find(' ');
					if (spacePos == std::string::npos)
						ctcpBits.push_back(std::pair<std::string, std::string> (ctcpSegment, ""));
					else
						ctcpBits.push_back(std::pair<std::string, std::string> (ctcpSegment.substr(0, spacePos), ctcpSegment.substr(spacePos + 1)));
				}
				ctcpPos = parsedLine[2].find((char)1, ctcpPos + 1);
			}
			// Call the appropriate hooks for the whole message and the CTCP bits based on whether it's being sent to a channel or to a user
			if (module->chanTypes.find(parsedLine[1][0]) != module->chanTypes.end()) {
				module->callChanMsgSendHook(id, parsedLine[1], ' ', parsedLine[2]);
				for (std::pair<std::string, std::string> ctcp : ctcpBits)
					module->callChanCTCPSendHook(id, parsedLine[1], ' ', ctcp.first, ctcp.second);
			} else if (module->chanTypes.find(parsedLine[1][1]) != module->chanTypes.end()) {
				bool chanCTCP = false;
				for (std::pair<std::string, char> status : module->prefixes) {
					if (status.second == parsedLine[1][0]) {
						chanCTCP = true;
						module->callChanMsgSendHook(id, parsedLine[1].substr(1), parsedLine[1][0], parsedLine[2]);
						for (std::pair<std::string, std::string> ctcp : ctcpBits)
							module->callChanCTCPSendHook(id, parsedLine[1].substr(1), parsedLine[1][0], ctcp.first, ctcp.second);
						break;
					}
				}
				if (!chanCTCP) {
					module->callUserMsgSendHook(id, parsedLine[1], parsedLine[2]);
					for (std::pair<std::string, std::string> ctcp : ctcpBits)
						module->callUserCTCPSendHook(id, parsedLine[1], ctcp.first, ctcp.second);
				}
			} else {
				module->callUserMsgSendHook(id, parsedLine[1], parsedLine[2]);
				for (std::pair<std::string, std::string> ctcp : ctcpBits)
					module->callUserCTCPSendHook(id, parsedLine[1], ctcp.first, ctcp.second);
			}
		}
	} else if (parsedLine[0] == "NOTICE") {
		size_t ctcpCount = 0;
		for (char msgChar : parsedLine[2]) {
			if (msgChar == (char)1)
				ctcpCount++;
		}
		// Detect whether the whole message is a CTCP
		if ((ctcpCount == 1 && parsedLine[2][0] == (char)1) || (ctcpCount == 2 && parsedLine[2][0] == (char)1 && parsedLine[2][parsedLine[2].size() - 1] == (char)1)) { // message is only a CTCP
			std::string ctcpMsg = parsedLine[2].substr(1);
			if (parsedLine[2][parsedLine[2].size() - 1] == (char)1)
				ctcpMsg = ctcpMsg.substr(0, ctcpMsg.size() - 1);
			size_t ctcpSpace = ctcpMsg.find(' ');
			std::string ctcp = ctcpMsg.substr(0, ctcpSpace);
			std::string params;
			if (ctcpSpace != std::string::npos)
				params = ctcpMsg.substr(ctcpSpace + 1);
			// Call the appropriate hook based on whether it's being sent to a channel or a user
			if (module->chanTypes.find(parsedLine[1][0]) != module->chanTypes.end())
				module->callChanCTCPReplySendHook(id, parsedLine[1], ' ', ctcp, params);
			else if (module->chanTypes.find(parsedLine[1][1]) != module->chanTypes.end()) {
				bool chanCTCP = false;
				for (std::pair<std::string, char> status : module->prefixes) {
					if (status.second == parsedLine[1][0]) {
						chanCTCP = true;
						 module->callChanCTCPReplySendHook(id, parsedLine[1].substr(1), parsedLine[1][0], ctcp, params);
						break;
					}
				}
				if (!chanCTCP)
					module->callUserCTCPReplySendHook(id, parsedLine[1], ctcp, params);
			} else
				module->callUserCTCPReplySendHook(id, parsedLine[1], ctcp, params);
		} else {
			// Either none or part of the message is CTCP; pull out the CTCP bits to call the appropriate hooks
			std::list<std::pair<std::string, std::string>> ctcpBits;
			size_t ctcpPos = 0;
			ctcpPos = parsedLine[2].find((char)1);
			while (ctcpPos != std::string::npos) {
				size_t startCTCP = ctcpPos;
				ctcpPos = parsedLine[2].find((char)1, ctcpPos + 1);
				std::string ctcpSegment = parsedLine[2].substr(startCTCP + 1, ctcpPos - (startCTCP + 1));
				if (!ctcpSegment.empty()) {
					size_t spacePos = ctcpSegment.find(' ');
					if (spacePos == std::string::npos)
						ctcpBits.push_back(std::pair<std::string, std::string> (ctcpSegment, ""));
					else
						ctcpBits.push_back(std::pair<std::string, std::string> (ctcpSegment.substr(0, spacePos), ctcpSegment.substr(spacePos + 1)));
				}
				ctcpPos = parsedLine[2].find((char)1, ctcpPos + 1);
			}
			// Call the appropriate hooks for the whole message and for the CTCP bits based on whether it's being sent to a channel or to a user
			if (module->chanTypes.find(parsedLine[1][0]) != module->chanTypes.end()) {
				module->callChanNoticeSendHook(id, parsedLine[1], ' ', parsedLine[2]);
				for (std::pair<std::string, std::string> ctcp : ctcpBits)
					module->callChanCTCPReplySendHook(id, parsedLine[1], ' ', ctcp.first, ctcp.second);
			} else if (module->chanTypes.find(parsedLine[1][1]) != module->chanTypes.end()) {
				bool chanCTCP = false;
				for (std::pair<std::string, char> status : module->prefixes) {
					if (status.second == parsedLine[1][0]) {
						chanCTCP = true;
						module->callChanNoticeSendHook(id, parsedLine[1].substr(1), parsedLine[1][0], parsedLine[2]);
						for (std::pair<std::string, std::string> ctcp : ctcpBits)
							module->callChanCTCPReplySendHook(id, parsedLine[1].substr(1), parsedLine[1][0], ctcp.first, ctcp.second);
						break;
					}
				}
				if (!chanCTCP) {
					module->callUserNoticeSendHook(id, parsedLine[1], parsedLine[2]);
					for (std::pair<std::string, std::string> ctcp : ctcpBits)
						module->callUserCTCPReplySendHook(id, parsedLine[1], ctcp.first, ctcp.second);
				}
			} else {
				module->callUserNoticeSendHook(id, parsedLine[1], parsedLine[2]);
				for (std::pair<std::string, std::string> ctcp : ctcpBits)
					module->callUserCTCPReplySendHook(id, parsedLine[1], ctcp.first, ctcp.second);
			}
		}
	}
	connection->sendData(message);
}

class Client : public Protocol {
	public:
		Client(std::string server, std::map<std::string, std::string> conf, std::string workDir, bool dumpLogs, unsigned short debug, Base* botptr);
		unsigned int apiVersion() = 0;
		void connectServer();
		void disconnectServer(std::string reason);
		bool isConnected();
		bool deadServer();
		bool isClient();
		
		void sendPrivMsg(const std::string& client, const std::string& target, const std::string& message);
		void sendNotice(const std::string& client, const std::string& target, const std::string& message);
		void sendCTCP(const std::string& client, const std::string& target, const std::string& ctcp, const std::string& params);
		void sendCTCPReply(const std::string& client, const std::string& target, const std::string& ctcp, const std::string& params);
		void setMode(const std::string& client, const std::string& target, const std::list<std::string>& setModes, const std::list<std::string>& remModes);
		void setSNOmask(const std::string& client, bool add, char snomask);
		void joinChan(const std::string& client, const std::string& channel, const std::string& key);
		void partChan( const std::string& client, const std::string& channel, const std::string& reason);
		void kickUser(const std::string& client, const std::string& channel, const std::string& user, const std::string& reason);
		std::string addClient(std::string& nick, std::string& ident, std::string& host, std::string& gecos);
		void removeClient(const std::string& client);
		void setTopic(const std::string& client, const std::string& channel, const std::string& topic);
		void inviteUser(const std::string& client, const std::string& channel, const std::string& user);
		void knockOnChannel(const std::string& client, const std::string& channel, const std::string& reason);
		void changeNick(const std::string& user, const std::string& newNick);
		void sendPing(const std::string& remoteServer);
		void operUp(const std::string& client, const std::string& usernameOrType, const std::string& password);
		void setXLine(const std::string& client, const std::string& lineType, const std::string& mask, time_t duration, const std::string& reason);
		void remXLine(const std::string& client, const std::string& lineType, const std::string& mask);
		void sendWallops(const std::string& client, const std::string& message);
		void sendOtherData(const std::string& client, const std::string& line);
		
		std::list<std::string> xLineTypes();
		std::set<std::string> chanTypes();
		std::list<std::string> chanListModes();
		std::list<std::string> chanParamModes();
		std::list<std::string> chanNoParamModes();
		std::list<std::pair<std::string, char>> chanPrefixes();
		std::pair<std::string, char> compareStatus(const std::string& status0, const std::string& status1);
		std::pair<std::string, char> compareStatus(const std::string& status0, char status1);
		std::pair<std::string, char> compareStatus(char status0, char status1);
		
		std::list<std::string> chanList();
		std::string chanTopic(const std::string& channel);
		std::string chanTopicSetter(const std::string& channel);
		time_t chanTimestamp(const std::string& channel);
		std::set<std::string> chanUsers(const std::string& channel);
		bool userInChan(const std::string& channel, const std::string& user);
		std::pair<std::string, char> userStatus(const std::string& channel, const std::string& user);
		bool userHasStatus(const std::string& channel, const std::string& user, const std::string& status);
		bool userHasStatus(const std::string& channel, const std::string& user, char status);
		bool userHasStatusOrGreater(const std::string& channel, const std::string& user, const std::string& status);
		bool userHasStatusOrGreater(const std::string& channel, const std::string& user, char status);
		std::list<std::string> chanModes(const std::string& channel);
		std::list<std::string> chanListModeList(const std::string& channel, const std::string& listMode);
		bool chanHasMode(const std::string& channel, const std::string& mode);
		std::string chanModeParam(const std::string& channel, const std::string& mode);
		
		std::list<std::string> clientList();
		std::string clientNick(const std::string& client);
		std::string userIdent(const std::string& user);
		std::string userHost(const std::string& user);
		std::set<std::string> userModes(const std::string& user);
		bool userHasMode(const std::string& user, const std::string& mode);
		std::set<char> userSNOmasks(const std::string& user);
		bool userHasSNOmask(const std::string& user, char snomask);
		std::set<std::string> userChans(const std::string& user);
		
		void processedOutChanMsg(const std::string& client, const std::string& channel, char status, const std::string& message);
		void processedOutUserMsg(const std::string& client, const std::string& nick, const std::string& message);
		void processedOutChanNotice(const std::string& client, const std::string& channel, char status, const std::string& message);
		void processedOutUserNotice(const std::string& client, const std::string& nick, const std::string& message);
		void processedOutChanCTCP(const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params);
		void processedOutUserCTCP(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params);
		void processedOutChanCTCPReply(const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params);
		void processedOutUserCTCPReply(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params);
	private:
		std::unordered_map<std::string, std::shared_ptr<User>> users;
		std::unordered_map<std::string, std::shared_ptr<Channel>> channels;
		std::unordered_map<std::string, std::shared_ptr<LocalClient>> connClients;
		bool floodControl;
		
		std::set<char> channelTypes;
		
		std::set<std::string> listModes;
		std::set<std::string> paramParamModes;
		std::set<std::string> paramModes;
		std::set<std::string> normalModes;
		std::list<std::pair<std::string, char>> prefixes;
		
		std::unordered_map<std::string, char> convertMode;
		std::unordered_map<char, std::string> convertChanMode, convertUserMode;
		unsigned int maxModes;
		
		void processIncoming(const std::string& client, const std::string& line);
		static std::vector<std::string> parseLine(const std::string& line);
		static std::tuple<std::string, std::string, std::string> parseHostmask(const std::string& hostmask);
		void registerLongMode(const std::string& modeName, char modeChar, bool chanMode);
		unsigned int currID;
		std::string newID();
		
		friend class LocalClient;
};

Client::Protocol(std::string server, std::map<std::string, std::string> conf, std::string workDir, bool dumpLogs, unsigned short debug, Base* botptr) : Protocol(server, conf, workDir, dumpLogs, debug, botptr), floodControl(true), currID(0) {}

unsigned int Client::apiVersion() {
	return 3000;
}

void Client::connectServer() {
	char floodSwitch = config["floodcontrol"][0];
	if (floodSwitch == 'n' || floodSwitch == 'N' || floodSwitch == '0' || floodSwitch == 'f' || floodSwitch == 'F')
		floodControl = false;
	else
		floodControl = true;
	if (config["sockettype"].empty())
		config["sockettype"] = config["0/sockettype"]; // It works because otherwise deadServer is true on startup and this module will be cleaned up
		// unless the type is there and invalid but valid in other clients, in which case, the user gets what he deserves.
	if (config["port"].empty())
		config["port"] = config["0/port"]; // Same as above.
	size_t i = 0;
	std::ostringstream clientNum;
	clientNum << i;
	while (!config[clientNum.str() + "/sockettype"].empty() && !config[clientNum.str() + "/port"].empty() && !config[clientNum.str() + "/id"].empty() && !config[clientNum.str() + "/nick"].empty() && !config[clientNum.str() + "/ident"].empty() && !config[clientNum.str() + "/gecos"].empty()) {
		std::shared_ptr<Socket> clientSocket = assignSocket(config[clientNum.str() + "/sockettype"]);
		if (clientSocket.get() == NULL)
			std::cerr << "The socket type for client " << i << " of " << serverName << " is not valid." << std::endl;
		else {
			bool sockSuccess = false;
			switch (clientSocket->connectServer(serverName, config[clientNum.str() + "/port"], config[clientNum.str() + "/bind"])) {
				case SOCKCONN_SUCCESS:
					sockSuccess = true;
					break;
				case SOCKCONN_HOST:
					std::cerr << "An error occurred getting the host for client " << i << " of server " << serverName << "." << std::endl;
					break;
				case SOCKCONN_FD:
					std::cerr << "An error occurred getting a file descriptor to establish a connection to server " << serverName << " for client " << i << "." << std::endl;
					break;
				case SOCKCONN_BIND:
					std::cerr << "An error occurred binding to the given address for the client " << i << " of server " << serverName << "." << std::endl;
					break;
				case SOCKCONN_CONNECT:
					std::cerr << "An error occurred establishing a connection for client " << i << " to server " << serverName << "." << std::endl;
					break;
				default:
					std::cerr << "An unknown socket error occurred for client " << i << " to server " << serverName << "." << std::endl;
			}
			if (sockSuccess) {
				std::shared_ptr<LocalClient> newClient (new LocalClient (config[clientNum.str() + "/id"], config[clientNum.str() + "/nick"], config[clientNum.str() + "/ident"], "", config[clientNum.str() + "/gecos"], this));
				connClients.insert(std::pair<std::string, std::shared_ptr<LocalClient>> (config[clientNum.str() + "/id"], newClient));
				newClient->connection = clientSocket;
				newClient->sendLine("CAP LS");
				if (!config[clientNum.str() + "/password"].empty())
					newClient->sendLine("PASS " + config[clientNum.str() + "/password"]);
				newClient->sendLine("NICK " + newClient->nick);
				newClient->sendLine("USER " + newClient->ident + " localhost " + serverName + " :" + newClient->gecos);
				newClient->altNick = config[clientNum.str() + "/altnick"];
				newClient->receiveThread = std::thread(&LocalClient::receive, newClient.get());
				if (floodControl) {
					newClient->sendThread = std::thread(&LocalClient::send, newClient.get());
					newClient->secondsThread = std::thread(&LocalClient::decrementSeconds, newClient.get());
				}
			}
		}
		i++;
		clientNum.str("");
		clientNum << i;
	}
	
	// This mapping of mode names and corresponding letters is borrowed from InspIRCd 2.0.
	registerLongMode("admin", 'a', true);
	registerLongMode("allowinvite", 'A', true);
	registerLongMode("antiredirect", 'L', false);
	registerLongMode("auditorium", 'u', true);
	registerLongMode("autoop", 'w', true);
	registerLongMode("ban", 'b', true);
	registerLongMode("banexception", 'e', true);
	registerLongMode("blockcaps", 'B', true);
	registerLongMode("blockcolor", 'c', true);
	registerLongMode("bot", 'B', false);
	registerLongMode("callerid", 'g', false);
	registerLongMode("censor", 'G', true);
	registerLongMode("cloak", 'x', false);
	registerLongMode("c_registered", 'r', true);
	registerLongMode("deaf", 'd', false);
	registerLongMode("deaf_commonchan", 'c', false);
	registerLongMode("delayjoin", 'D', true);
	registerLongMode("delaymsg", 'd', true);
	registerLongMode("exemptchanops", 'X', true);
	registerLongMode("filter", 'g', true);
	registerLongMode("flood", 'f', true);
	registerLongMode("founder", 'q', true);
	registerLongMode("halfop", 'h', true);
	registerLongMode("helpop", 'h', false);
	registerLongMode("hidechans", 'I', false);
	registerLongMode("hideoper", 'H', false);
	registerLongMode("history", 'H', true);
	registerLongMode("invex", 'I', true);
	registerLongMode("invisible", 'i', false);
	registerLongMode("inviteonly", 'i', true);
	registerLongMode("joinflood", 'j', true);
	registerLongMode("key", 'k', true);
	registerLongMode("kicknorejoin", 'J', true);
	registerLongMode("limit", 'l', true);
	registerLongMode("moderated", 'm', true);
	registerLongMode("namebase", 'Z', true);
	registerLongMode("nickflood", 'F', true);
	registerLongMode("noctcp", 'C', true);
	registerLongMode("noextmsg", 'n', true);
	registerLongMode("nokick", 'Q', true);
	registerLongMode("noknock", 'K', true);
	registerLongMode("nonick", 'N', true);
	registerLongMode("nonotice", 'T', true);
	registerLongMode("official-join", 'Y', true);
	registerLongMode("op", 'o', true);
	registerLongMode("oper", 'o', false);
	registerLongMode("operonly", 'O', true);
	registerLongMode("operprefix", 'y', true);
	registerLongMode("permanent", 'P', true);
	registerLongMode("private", 'p', true);
	registerLongMode("redirect", 'L', true);
	registerLongMode("regdeaf", 'R', false);
	registerLongMode("reginvite", 'R', true);
	registerLongMode("regmoderated", 'M', true);
	registerLongMode("secret", 's', true);
	registerLongMode("servprotect", 'k', false);
	registerLongMode("showwhois", 'W', false);
	registerLongMode("snomask", 's', false);
	registerLongMode("sslonly", 'z', true);
	registerLongMode("stripcolor", 'S', true);
	registerLongMode("topiclock", 't', true);
	registerLongMode("u_censor", 'G', false);
	registerLongMode("u_register", 'r', false);
	registerLongMode("u_stripcolor", 'S', false);
	registerLongMode("voice", 'v', true);
	registerLongMode("wallops", 'w', false);
}

void Client::disconnectServer(std::string reason) {
	// First send QUIT to the server from all of the clients
	for (std::pair<std::string, std::shared_ptr<LocalClient>> client : connClients)
		client.second->sendLine("QUIT :Disconnecting server");
	// Then let all the clients' threads finish.
	// This is done separately so that if more than one client takes a bit to flush, we're not waiting that much longer
	// for both of them rather than just one.
	for (std::pair<std::string, std::shared_ptr<LocalClient>> client : connClients) {
		if (client.second->receiveThread.joinable())
			client.second->receiveThread.join();
		if (client.second->sendThread.joinable())
			client.second->sendThread.join();
		if (client.second->secondsThread.joinable())
			client.second->secondsThread.join();
	}
}

bool Client::isConnected() {
	for (std::pair<std::string, std::shared_ptr<LocalClient>> client : connClients) {
		if (!client.second->connection->isConnected())
			return false;
	}
	return true;
}

bool Client::deadServer() {
	return connClients.empty();
}

bool Client::isClient() {
	return true;
}

void Client::sendPrivMsg(const std::string& client, const std::string& target, const std::string& message) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	if (channelTypes.find(target[0]) != channelTypes.end())
		callChanMsgOutHook(client, target, ' ', message);
	else if (channelTypes.find(target[1]) != channelTypes.end()) {
		bool chanMsg = false;
		for (std::pair<std::string, char> status : prefixes) {
			if (status.second == target[0]) {
				callChanMsgOutHook(client, target.substr(1), target[0], message);
				chanMsg = true;
				break;
			}
		}
		if (!chanMsg)
			callUserMsgOutHook(client, target, message);
	} else
		callUserMsgOutHook(client, target, message);
}

void Client::sendNotice(const std::string& client, const std::string& target, const std::string& message) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	if (channelTypes.find(target[0]) != channelTypes.end())
		callChanNoticeOutHook(client, target, ' ', message);
	else if (channelTypes.find(target[1]) != channelTypes.end()) {
		bool chanMsg = false;
		for (std::pair<std::string, char> status : prefixes) {
			if (status.second == target[0]) {
				callChanNoticeOutHook(client, target.substr(1), target[0], message);
				chanMsg = true;
				break;
			}
		}
		if (!chanMsg)
			callUserNoticeOutHook(client, target, message);
	} else
		callUserNoticeOutHook(client, target, message);
}

void Client::sendCTCP(const std::string& client, const std::string& target, const std::string& ctcp, const std::string& params) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	if (channelTypes.find(target[0]) != channelTypes.end())
		callChanCTCPOutHook(client, target, ' ', ctcp, params);
	else if (channelTypes.find(target[1]) != channelTypes.end()) {
		bool chanCTCP = false;
		for (std::pair<std::string, char> status : prefixes) {
			if (status.second == target[0]) {
				callChanCTCPOutHook(client, target.substr(1), target[0], ctcp, params);
				chanCTCP = true;
				break;
			}
		}
		if (!chanCTCP)
			callUserCTCPOutHook(client, target, ctcp, params);
	} else
		callUserCTCPOutHook(client, target, ctcp, params);
}

void Client::sendCTCPReply(const std::string& client, const std::string& target, const std::string& ctcp, const std::string& params) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	if (channelTypes.find(target[0]) != channelTypes.end())
		callChanCTCPReplyOutHook(client, target, ' ', ctcp, params);
	else if (channelTypes.find(target[1]) != channelTypes.end()) {
		bool chanCTCP = false;
		for (std::pair<std::string, char> status : prefixes) {
			if (status.second == target[0]) {
				callChanCTCPReplyOutHook(client, target.substr(1), target[0], ctcp, params);
				chanCTCP = true;
				break;
			}
		}
		if (!chanCTCP)
			callUserCTCPReplyOutHook(client, target, ctcp, params);
	} else
		callUserCTCPReplyOutHook(client, target, ctcp, params);
}

void Client::setMode(const std::string& client, const std::string& target, const std::list<std::string>& setModes, const std::list<std::string>& remModes) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	std::list<std::string> modesToSet = setModes, modesToRemove = remModes;
	unsigned int modeCount = 0;
	std::list<char> setModesChar, remModesChar;
	std::string params = "";
	while (!modesToSet.empty() && modeCount < maxModes) {
		std::string mode = modesToSet.front();
		size_t paramPos = mode.find('=');
		std::string param ("");
		if (paramPos != std::string::npos) {
			param = mode.substr(paramPos + 1);
			mode = mode.substr(0, paramPos);
		}
		std::unordered_map<std::string, char>::iterator convIter = convertMode.find(mode);
		if (convIter != convertMode.end()) {
			setModesChar.push_back(convIter);
			params += " " + param;
		}
		modesToSet.erase(modesToSet.begin());
		modeCount++;
	}
	while (!modesToRemove.empty() && modeCount < maxModes) {
		std::string mode = modesToRemove.front();
		size_t paramPos = mode.find('=');
		std::string param ("");
		if (paramPos != std::string::npos) {
			param = mode.substr(paramPos + 1);
			mode = mode.substr(0, paramPos);
		}
		std::unordered_map<std::string, char>::iterator convIter = convertMode.find(mode);
		if (convIter != convertMode.end()) {
			remModesChar.push_back(convIter);
			params += " " + param;
		}
		modesToRemove.erase(modesToRemove.begin());
		modeCount++;
	}
	std::string modesStr = "";
	if (!setModesChar.empty()) {
		modesStr += "+";
		for (char modeChar : setModesChar)
			modesStr += modeChar;
	}
	if (!remModesChar.empty()) {
		modesStr += "-";
		for (char modeChar : remModesChar)
			modesStr += modeChar;
	}
	if (modesStr.empty())
		return;
	clientIter->second->sendLine("MODE " + target + " " + modesStr + params);
	if (modeCount >= maxModes)
		setMode(client, target, modesToSet, modesToRemove); // Send the rest of the modes as another line
}

void Client::setSNOmask(const std::string& client, bool add, char snomask) {
	std::unordered_map<std::string, char>::iterator convIter = convertMode.find("snomask");
	if (convIter == convertMode.end())
		return;
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("MODE " + clientIter->second->nick + " +" + std::string(convIter->second) + " " + (add ? "+" : "-") + std::string(snomask));
}

void Client::joinChan(const std::string& client, const std::string& channel, const std::string& key) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	std::string joinLine = "JOIN " + channel;
	if (!key.empty())
		joinLine += " " + key;
	clientIter->second->sendLine(joinLine);
}

void Client::partChan( const std::string& client, const std::string& channel, const std::string& reason) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("PART " + channel + " :" + reason);
}

void Client::kickUser(const std::string& client, const std::string& channel, const std::string& user, const std::string& reason) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("KICK " + channel + " " + user + " :" + reason);
}

std::string Client::addClient(std::string& nick, std::string& ident, std::string& host, std::string& gecos) {
	std::shared_ptr<Socket> clientSocket = assignSocket(config["sockettype"]);
	if (clientSocket.get() == NULL)
		return ""; // NOPE
	if (clientSocket->connectServer(serverName, config["port"], config["bind"]) == SOCKCONN_SUCCESS) {
		std::string clientID = newID();
		std::shared_ptr<LocalClient> newClient (new LocalClient (clientID, nick, ident, host, gecos, this));
		connClients.insert(std::pair<std::string, std::shared_ptr<LocalClient>> (clientID, newClient));
		newClient->connection = clientSocket;
		newClient->sendLine("CAP LS");
		if (!config["password"].empty())
			newClient->sendLine("PASS " + config["password"]);
		newClient->sendLine("NICK " + nick);
		newClient->sendLine("USER " + ident + " localhost " + serverName + " :" + gecos);
		newClient->receiveThread = std::thread(&LocalClient::receive, newClient);
		if (floodControl) {
			newClient->sendThread = std::thread(&LocalClient::send, newClient);
			newClient->secondsThread = std::thread(&LocalClient::decrementSeconds, newClient);
		}
		return clientID;
	} else
		return "";
}

void Client::removeClient(const std::string& client) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("QUIT :Disconnecting client");
	if (clientIter->second->receiveThread.joinable())
		clientIter->second->receiveThread.join();
	if (clientIter->second->sendThread.joinable())
		clientIter->second->sendThread.join();
	if (clientIter->second->secondsThread.joinable())
		clientIter->second->secondsThread.join();
	connClients.erase(clientIter);
}

void Client::setTopic(const std::string& client, const std::string& channel, const std::string& topic) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("TOPIC " + channel + " :" + topic);
}

void Client::inviteUser(const std::string& client, const std::string& channel, const std::string& user) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("INVITE " + user + " " + channel);
}

void Client::knockOnChannel(const std::string& client, const std::string& channel, const std::string& reason) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("KNOCK " + channel + " :" + reason);
}

void Client::changeNick(const std::string& user, const std::string& newNick) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(user);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("NICK " + newNick);
}

void Client::sendPing(const std::string& remoteServer) {
	if (connClients.empty())
		return;
	connClients.begin()->second->sendLine("PING :" + remoteServer);
}

void Client::operUp(const std::string& client, const std::string& usernameOrType, const std::string& password) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("OPER " + usernameOrType + " " + password);
}

void Client::setXLine(const std::string& client, const std::string& lineType, const std::string& mask, time_t duration, const std::string& reason) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	std::ostringstream lineToSend;
	if (lineType.size() > 2)
		lineToSend << lineType;
	else
		lineToSend << lineType << "LINE";
	lineToSend << " " << mask << " " << duration << " :" << reason;
	clientIter->second->sendLine(lineToSend.str());
}

void Client::remXLine(const std::string& client, const std::string& lineType, const std::string& mask) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	if (lineType.size() > 2)
		clientIter->second->sendLine(lineType + " " + mask);
	else
		clientIter->second->sendLine(lineType + "LINE " + mask);
}

void Client::sendWallops(const std::string& client, const std::string& message) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("WALLOPS :" + message);
}

void Client::sendOtherData(const std::string& client, const std::string& line) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine(line);
}

std::list<std::string> Client::xLineTypes() {
	return std::list<std::string> { "G", "K", "Z", "E", "SHUN" };
}

std::set<std::string> Client::chanTypes() {
	return channelTypes;
}

std::list<std::string> Client::chanListModes() {
	std::list<std::string> modes;
	for (std::string mode : listModes)
		modes.push_back(mode);
	return modes;
}

std::list<std::string> Client::chanParamModes() {
	std::list<std::string> modes;
	for (std::string mode : paramModes)
		modes.push_back(mode);
	for (std::string mode : paramParamModes)
		modes.push_back(mode);
	return modes;
}

std::list<std::string> Client::chanNoParamModes() {
	std::list<std::string> modes;
	for (std::string mode : normalModes)
		modes.push_back(mode);
	return modes;
}

std::list<std::pair<std::string, char>> Client::chanPrefixes() {
	return prefixes;
}

std::pair<std::string, char> Client::compareStatus(const std::string& status0, const std::string& status1) {
	for (std::pair<std::string, char> status : prefixes) {
		if (status0 == status.first || status1 == status.first)
			return status;
	}
	return std::pair<std::string, char> ("", ' ');
}

std::pair<std::string, char> Client::compareStatus(const std::string& status0, char status1) {
	for (std::pair<std::string, char> status : prefixes) {
		if (status0 == status.first || status1 == status.second)
			return status;
	}
	return std::pair<std::string, char> ("", ' ');
}

std::pair<std::string, char> Client::compareStatus(char status0, char status1) {
	for (std::pair<std::string, char> status : prefixes) {
		if (status0 == status.second || status1 == status.second)
			return status;
	}
	return std::pair<std::string, char> ("", ' ');
}

std::list<std::string> Client::chanList() {
	std::list<std::string> inChans;
	for (std::pair<std::string, std::shared_ptr<Channel>> channel : channels)
		inChans.push_back(channel.first);
	return inChans;
}

std::string Client::chanTopic(const std::string& channel) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return "";
	return chanIter->second->topic;
}

std::string Client::chanTopicSetter(const std::string& channel) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return "";
	return chanIter->second->topicSetter;
}

time_t Client::chanTimestamp(const std::string& channel) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return 0;
	return chanIter->second->timestamp;
}

std::set<std::string> Client::chanUsers(const std::string& channel) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return std::set<std::string> ();
	return chanIter->second->users;
}

bool Client::userInChan(const std::string& channel, const std::string& user) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return false;
	return (chanIter->second->users.find(user) != chanIter->second->users.end());
}

std::pair<std::string, char> Client::userStatus(const std::string& channel, const std::string& user) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return std::pair<std::string, char> ("", ' ');
	std::unordered_map<std::string, char>::iterator statusIter = chanIter->second->statuses.find(user);
	if (statusIter == chanIter->second->statuses.end())
		return std::pair<std::string, char> ("", ' ');
	for (std::pair<std::string, char> status : prefixes) {
		if (status.second == statusIter->second)
			return status;
	}
	// It should never be the case that we get down here, but...
	//  1. it avoids compiler warnings, and
	//  2. if some weird bug does happen, we do still have to return something.
	return std::pair<std::string, char> ("", ' ');
}

bool Client::userHasStatus(const std::string& channel, const std::string& user, const std::string& status) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return false;
	std::unordered_map<std::string, char>::iterator statusIter = chanIter->second->statuses.find(user);
	if (statusIter == chanIter->second->statuses.end())
		return false;
	for (std::pair<std::string, char> prefix : prefixes) {
		if (prefix.first == status)
			return (statusIter->second == prefix.second);
	}
	return false;
}

bool Client::userHasStatus(const std::string& channel, const std::string& user, char status) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return false;
	std::unordered_map<std::string, char>::iterator statusIter = chanIter->second->statuses.find(user);
	if (statusIter == chanIter->second->statuses.end())
		return false;
	return (statusIter->second == status);
}

bool Client::userHasStatusOrGreater(const std::string& channel, const std::string& user, const std::string& status) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return false;
	std::unordered_map<std::string, char>::iterator statusIter = chanIter->second->statuses.find(user);
	if (statusIter == chanIter->second->statuses.end())
		return false;
	for (std::pair<std::string, char> prefix : prefixes) {
		if (statusIter->second == prefix.second)
			return true;
		if (prefix.first == status)
			return false;
	}
	return false;
}

bool Client::userHasStatusOrGreater(const std::string& channel, const std::string& user, char status) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return false;
	std::unordered_map<std::string, char>::iterator statusIter = chanIter->second->statuses.find(user);
	if (statusIter == chanIter->second->statuses.end())
		return false;
	for (std::pair<std::string, char> prefix : prefixes) {
		if (statusIter->second == prefix.second)
			return true;
		if (prefix.second == status)
			return false;
	}
	return false;
}

std::list<std::string> Client::chanModes(const std::string& channel) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return std::list<std::string> ();
	return chanIter->second->modes;
}

std::list<std::string> Client::chanListModeList(const std::string& channel, const std::string& listMode) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return std::list<std::string> ();
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator listIter = chanIter->second->listModes.find(listMode);
	if (listIter == chanIter->second->listModes.end())
		return std::list<std::string> ();
	return listIter->second;
}

bool Client::chanHasMode(const std::string& channel, const std::string& mode) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return;
	for (std::string chanMode : chanIter->second->modes) {
		if (chanMode == mode)
			return true;
	}
	return false;
}

std::string Client::chanModeParam(const std::string& channel, const std::string& mode) {
	std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(channel);
	if (chanIter == channels.end())
		return;
	for (std::string chanMode : chanIter->second->modes) {
		size_t equals = chanMode.find('=');
		if (equals == std::string::npos)
			continue;
		if (chanMode.substr(0, equals) == mode)
			return chanMode.substr(equals + 1);
	}
	return "";
}

std::list<std::string> Client::clientList() {
	std::list<std::string> clients;
	for (std::pair<std::string, std::shared_ptr<LocalClient>> client : connClients)
		clients.push_back(client.first);
	return clients;
}

std::string Client::clientNick(const std::string& client) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return "";
	return clientIter->second->nick;
}

std::string Client::userIdent(const std::string& user) {
	User* theUser;
	std::unordered_map<std::string, std::shared_ptr<User>>::iterator userIter = users.find(user);
	if (userIter == users.end()) {
		std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(user);
		if (clientIter == connClients.end())
			return "";
		theUser = static_cast<User*> clientIter->second.get();
	} else
		theUser = userIter->second.get();
	return theUser->ident;
}

std::string Client::userHost(const std::string& user) {
	User* theUser;
	std::unordered_map<std::string, std::shared_ptr<User>>::iterator userIter = users.find(user);
	if (userIter == users.end()) {
		std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(user);
		if (clientIter == connClients.end())
			return "";
		theUser = static_cast<User*> clientIter->second.get();
	} else
		theUser = userIter->second.get();
	return theUser->host;
}

std::set<std::string> Client::userModes(const std::string& user) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(user);
	if (clientIter == connClients.end())
		return std::set<std::string> ();
	return clientIter->second->modes;
}

bool Client::userHasMode(const std::string& user, const std::string& mode) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(user);
	if (clientIter == connClients.end())
		return false;
	return (clientIter->second->modes.find(mode) != clientIter->second->modes.end());
}

std::set<char> Client::userSNOmasks(const std::string& user) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(user);
	if (clientIter == connClients.end())
		return std::set<char>();
	return clientIter->second->snomasks;
}

bool Client::userHasSNOmask(const std::string& user, char snomask) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(user);
	if (clientIter == connClients.end())
		return false;
	return (clientIter->second->snomasks.find(snomask) != clientIter->second->snomasks.end());
}

std::set<std::string> Client::userChans(const std::string& user) {
	User* theUser;
	std::unordered_map<std::string, std::shared_ptr<User>>::iterator userIter = users.find(user);
	if (userIter == users.end()) {
		std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(user);
		if (clientIter == connClients.end())
			return std::set<std::string> ();
		theUser = static_cast<User*> clientIter->second.get();
	} else
		theUser = userIter->second.get();
	return theUser->channels;
}

void Client::processedOutChanMsg(const std::string& client, const std::string& channel, char status, const std::string& message) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	// Make sure the client in question is still around, in case the modules took forever or something and the client disconnected in the meantime
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("PRIVMSG " + (status == ' ' ? "" : std::string(status)) + channel + " :" + message);
}

void Client::processedOutUserMsg(const std::string& client, const std::string& nick, const std::string& message) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("PRIVMSG " + nick + " :" + message);
}

void Client::processedOutChanNotice(const std::string& client, const std::string& channel, char status, const std::string& message) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("NOTICE " + (status == ' ' ? "" : std::string(status)) + channel + " :" + message);
}

void Client::processedOutUserNotice(const std::string& client, const std::string& nick, const std::string& message) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("NOTICE " + nick + " :" + message);
}

void Client::processedOutChanCTCP(const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("PRIVMSG " + (status == ' ' ? "" : std::string(status)) + channel + " :\x01" + ctcp + (params == "" ? "" : (" " + params)) + "\x01");
}

void Client::processedOutUserCTCP(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("PRIVMSG " + nick + " :\x01" + ctcp + (params == "" ? "" : (" " + params)) + "\x01");
}

void Client::processedOutChanCTCPReply(const std::string& client, const std::string& channel, char status, const std::string& ctcp, const std::string& params) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("NOTICE " + (status == ' ' ? "" : std::string(status)) + channel + " :\x01" + ctcp + (params == "" ? "" : (" " = params)) + "\x01");
}

void Client::processedOutUserCTCPReply(const std::string& client, const std::string& nick, const std::string& ctcp, const std::string& params) {
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (clientIter == connClients.end())
		return;
	clientIter->second->sendLine("NOTICE " + nick + " :\x01" + ctcp + (params == "" ? "" : (" " + params)) + "\x01");
}

void Client::processIncoming(const std::string& client, const std::string& line) {
	std::vector<std::string> parsedLine = parseLine(line);
	std::unordered_map<std::string, std::shared_ptr<LocalClient>>::iterator clientIter = connClients.find(client);
	if (parsedLine[0] == "PING") {
		clientIter->second->sendLine("PONG :" + parsedLine[1]);
		callServerPingHook(parsedLine[1]);
	} else if (parsedLine[1] == "PONG")
		callServerPongHook(parsedLine[2]);
	else if (parsedLine[1] == "CAP") {
		if (parsedLine[3] == "LS") {
			std::list<std::string> capList;
			std::string buffer;
			for (char capChar : parsedLine[4]) {
				if (capChar == ' ') {
					capList.push_back(buffer);
					buffer.clear();
				} else
					buffer += capChar;
			}
			if (!buffer.empty())
				capList.push_back(buffer);
			std::string capResponse;
			for (std::string cap : capList) {
				if (cap == "multi-prefix")
					capResponse += " multi-prefix";
				else if (cap == "userhost-in-names")
					capResponse += " userhost-in-names";
			}
			callServerCapHook(client, "LS", parsedLine[4]);
			std::this_thread::sleep_for(std::chrono::seconds(1)); // Give modules a chance to react before we process ACK, etc. which will send CAP END
			if (!capResponse.empty())
				clientIter->second->sendLine("CAP REQ :" + capResponse.substr(1));
			else
				clientIter->second->sendLine("CAP END");
		} else if ((parsedLine[3] == "ACK" || parsedLine[3] == "NAK") && !clientIter->second->registered) {
			callServerCapHook(client, parsedLine[3], parsedLine[4]);
			clientIter->second->sendLine("CAP END");
		} else
			callServerCapHook(client, parsedLine[3], parsedLine[4]);
	} else if (parsedLine[1] == "001") { // registered
		clientIter->second->registered = true;
		callNumericHook(client, "001", std::vector<std::string> (parsedLine.begin() + 2, parsedLine.end()));
		std::unordered_map<std::string, char>::iterator convIter = convertMode.find("bot");
		if (convIter != convertMode.end())
			clientIter->second->sendLine("MODE " + clientIter->second->nick + " +" + std::string(convIter->second));
	} else if (parsedLine[1] == "005") { // supports
		for (std::vector<std::string>::iterator tokenIter = parsedLine.begin() + 3; tokenIter != parsedLine.end(); ++tokenIter) {
			if ((*tokenIter).substr(0, 7) == "PREFIX=") {
				std::string prefixes = (*tokenIter).substr(8);
				std::string modes, symbols;
				std::string::iterator prefixIter = prefixes.begin();
				while ((*prefixIter) != ')') {
					modes += *prefixIter;
					++prefixIter;
				}
				++prefixIter; // Skip over the close-paren
				while (prefixIter != prefixes.end()) {
					symbols += prefixIter;
					++prefixIter;
				}
				for (size_t i = 0; i < modes.size(); i++) {
					std::unordered_map<char, std::string>::iterator convIter = convertChanMode.find(modes[i]);
					if (convIter != convertChanMode.end())
						prefixes.push_back(std::pair<std::string, char> (convertChanMode[modes[i]], symbols[i]));
					else {
						std::cerr << "Config error: mode " << modes[i] << " was not able to be converted to a long-name mode!  This may or may not be a major problem." << std::endl;
						prefixes.push_back(std::pair<std::string, char> (std::string(modes[i]), symbols[i]));
					}
				}
			} else if ((*tokenIter).substr(0, 6) == "MODES=") {
				std::istringstream maxModesStr ((*tokenIter).substr(6));
				maxModesStr >> maxModes;
			} else if ((*tokenIter).substr(0, 10) == "CHANMODES=") {
				unsigned int section = 0;
				for (char mode : (*tokenIter).substr(10)) {
					if (mode == ',')
						section++;
					else {
						std::string addMode;
						std::unordered_map<char, std::string>::iterator convIter = convertChanMode.find(mode);
						if (convIter == convertChanMode.end())
							addMode = mode;
						else
							addMode = convIter->second;
						switch(section) {
							case 0:
								listModes.insert(addMode);
								break;
							case 1:
								paramParamModes.insert(addMode);
								break;
							case 2:
								paramModes.insert(addMode);
								break;
							case 3:
								normalModes.insert(addMode);
						}
					}
				}
			} else if ((*tokenIter).substr(0, 10) == "CHANTYPES=") {
				for (char type : (*tokenIter).substr(10))
					chanTypes.insert(type);
			}
		}
		callNumericHook(client, "005", std::vector<std::string> (parsedLine.begin() + 2, parsedLine.end()));
	} else if (parsedLine[1] == "433" && !clientIter->second->registered && !clientIter->second->triedAlt) {
		clientIter->second->sendLine("NICK " + clientIter->second->altNick);
		clientIter->second->triedAlt = true;
		callNumericHook(client, "433", std::vector<std::string> (parsedLine.begin() + 2, parsedLine.end()));
	} else if (parsedLine[1] == "352") { // WHO reply
		std::unordered_map<std::string, std::shared_ptr<User>>::iterator userIter = users.find(parsedLine[7]);
		if (userIter != users.end()) { // If it is, it's probably a client, which can handle itself.
			userIter->second->ident = parsedLine[4];
			userIter->second->host = parsedLine[5];
		}
		callNumericHook(client, "352", std::vector<std::string> (parsedLine.begin() + 2, parsedLine.end()));
	} else if (parsedLine[1] == "353") { // NAMES reply
		std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(parsedLine[4]);
		if (chanIter != channels.end()) { // If it's not in channels, it's possible a module requested NAMES on a channel we're not in
			if (chanIter->second.namesComplete) {
				chanIter->second->statuses.clear();
				for (std::string user : chanIter->second->users)
					users.find(user)->second->channels.erase(chanIter->first);
				chanIter->second->users.clear();
				chanIter->second->namesComplete = false;
				chanIter->second->whoCalled = false;
			}
			std::list<std::string> names;
			std::string buffer;
			for (char nameChar : parsedLine[5]) {
				if (nameChar == ' ') {
					names.push_back(buffer);
					buffer = "";
				} else
					buffer += nameChar;
			}
			if (!buffer.empty())
				names.push_back(buffer);
			for (std::string name : names) {
				std::string nick, ident, host;
				std::list<char> statuses;
				for (std::pair<std::string, char> status : prefixes) {
					if (name[0] == status.second) {
						statuses.push_back(status.second);
						name = name.substr(1);
					}
				}
				std::tie(nick, ident, host) = parseHostmask(name);
				if ((ident.empty() || host.empty()) && !chanIter->second->whoCalled) {
					clientIter->second->sendLine("WHO " + chanIter->first);
					chanIter->second->whoCalled = true;
				}
				std::unordered_map<std::string, std::shared_ptr<User>>::iterator userIter = users.find(nick);
				if (userIter == users.end())
					userIter = users.insert(std::pair<std::string, std::shared_ptr<User>> (nick, new User (nick, ident, host))).first;
				else {
					userIter->second->ident = ident;
					userIter->second->host = host;
				}
				userIter->second->channels.insert(chanIter->first);
				chanIter->second->users.insert(nick);
				if (!statuses.empty())
					chanIter->second->statuses.insert(std::pair<std::string, std::list<char>> (nick, statuses));
			}
		}
		callNumericHook(client, "353", std::vector<std::string> (parsedLine.begin() + 2, parsedLine.end()));
	} else if (parsedLine[1] == "366") { // end of NAMES reply
		std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(parsedLine[3]);
		if (chanIter != channels.end())
			chanIter->second->namesComplete = true;
		callNumericHook(client, "366", std::vector<std::string> (parsedLine.begin() + 2, parsedLine.end()));
	} else if (parsedLine[1] == "332") { // channel topic
		std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(parsedLine[3]);
		if (chanIter != channels.end())
			chanIter->second->topic = parsedLine[4];
		callNumericHook(client, "332", std::vector<std::string> (parsedLine.begin() + 2, parsedLine.end()));
	} else if (parsedLine[1] == "333") { // topic setter
		std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(parsedLine[3]);
		if (chanIter != channels.end())
			chanIter->second->topicSetter = parsedLine[4];
		callNumericHook(client, "333", std::vector<std::string> (parsedLine.begin() + 2, parsedLine.end()));
	} else if (parsedLine[1] == "324") { // channel modes
		std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(parsedLine[3]);
		if (chanIter != channels.end()) {
			chanIter->second->modes.clear();
			size_t currParam = 5;
			for (char mode : parsedLine[4]) {
				if (mode == '+')
					continue;
				std::string longMode = mode;
				std::unordered_map<char, std::string> convIter = convertChanMode.find(mode);
				if (convIter != convertChanMode.end())
					longMode = convIter->second;
				if (normalModes.find(longMode) == normalModes.end())
					longMode += ("=" + parsedLine[currParam++]);
				chanIter->second->modes.push_back(longMode);
			}
		}
		callNumericHook(client, "324", std::vector<std::string> (parsedLine.begin() + 2, parsedLine.end()));
	} else if (parsedLine[1] == "329") { // channel timestamp
		std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(parsedLine[3]);
		if (chanIter != channels.end()) {
			std::istringstream tsStr (parsedLine[4]);
			tsStr >> chanIter->second->timestamp;
		}
		callNumericHook(client, "329", std::vector<std::string> (parsedLine.begin() + 2, parsedLine.end()));
	} else if (parsedLine[1] == "367") { // ban list
		std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(parsedLine[3]);
		if (chanIter != channels.end()) {
			if (chanIter->second->listComplete["ban"]) {
				chanIter->second->listComplete["ban"] = false;
				chanIter->second->listModes["ban"].clear();
			}
			chanIter->second->listModes["ban"].push_back(parsedLine[4]);
		}
		callNumericHook(client, "367", std::vector<std::string> (parsedLine.begin() + 2, parsedLine.end()));
	} else if (parsedLine[1] == "368") { // end of ban list
		std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(parsedLine[3]);
		if (chanIter != channels.end())
			chanIter->second->listComplete["ban"] = true;
		callNumericHook(client, "368", std::vector<std::string> (parsedLine.begin() + 2, parsedLine.end()));
	} else if (parsedLine[1] == "348") { // ban exception list
		std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(parsedLine[3]);
		if (chanIter != channels.end()) {
			if (chanIter->second->listComplete["banexception"]) {
				chanIter->second->listComplete["banexception"] = false;
				chanIter->second->listModes["banexception"].clear();
			}
			chanIter->second->listModes["banexception"].push_back(parsedLine[4]);
		}
		callNumericHook(client, "348", std::vector<std::string> (parsedLine.begin() + 2, parsedLine.end()));
	} else if (parsedLine[1] == "349") { // end of ban exception list
		std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(parsedLine[3]);
		if (chanIter != channels.end())
			chanIter->second->listComplete["banexception"] = true;
		callNumericHook(client, "349", std::vector<std::string> (parsedLine.begin() + 2, parsedLine.end()));
	} else if (parsedLine[1] == "346") { // invite exception list
		std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(parsedLine[3]);
		if (chanIter != channels.end()) {
			if (chanIter->second->listComplete["invex"]) {
				chanIter->second->listComplete["invex"] = false;
				chanIter->second->listModes["invex"].clear();
			}
			chanIter->second->listModes["invex"].push_back(parsedLine[4]);
		}
		callNumericHook(client, "346", std::vector<std::string> (parsedLine.begin() + 2, parsedLine.end()));
	} else if (parsedLine[1] == "347") { // end of invite exception list
		std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(parsedLine[3]);
		if (chanIter != channels.end())
			chanIter->second->listComplete["invex"] = true;
		callNumericHook(client, "347", std::vector<std::string> (parsedLine.begin() + 2, parsedLine.end()));
	} else if (parsedLine[1] == "941") { // spamfilter list
		std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(parsedLine[3]);
		if (chanIter != channels.end()) {
			if (chanIter->second->listComplete["filter"]) {
				chanIter->second->listComplete["filter"] = false;
				chanIter->second->listModes["filter"].clear();
			}
			chanIter->second->listModes["filter"].push_back(parsedLine[4]);
		}
		callNumericHook(client, "941", std::vector<std::string> (parsedLine.begin() + 2, parsedLine.end()));
	} else if (parsedLine[1] == "940") { // end of spamfilter list
		std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(parsedLine[3]);
		if (chanIter != channels.end())
			chanIter->second->listComplete["filter"] = true;
		callNumericHook(client, "940", std::vector<std::string> (parsedLine.begin() + 2, parsedLine.end()));
	} else if (parsedLine[1] == "710") { // user knocked on channel
		std::string nick;
		std::tie(nick, std::ignore, std::ignore) = parseHostmask(parsedLine[4]);
		callChanKnockHook(parsedLine[3], parsedLine[4], parsedLine[5]);
	} else if (parsedLine[1].size() == 3 && (parsedLine[1][0] >= '0' && parsedLine[1][0] <= '9') && (parsedLine[1][1] >= '0' && parsedLine[1][1] <= '9') && (parsedLine[1][2] >= '0' &7 parsedLine[1][0] <= '9'))
		callNumericHook(client, parsedLine[1], std::vector<std::string> (parsedLine.begin() + 2, parsedLine.end()));
	else if (parsedLine[1] == "PRIVMSG") {
		std::string hostmask = parsedLine[0];
		if (hostmask[0] == ':')
			hostmask = hostmask.substr(1);
		std::string nick;
		std::tie(nick, std::ignore, std::ignore) = parseHostmask(hostmask);
		size_t ctcpCount = 0;
		for (char msgChar : parsedLine[3]) {
			if (msgChar == (char)1)
				ctcpCount++;
		}
		// Detect whether the whole message is a CTCP
		if ((ctcpCount == 1 && parsedLine[3][0] == (char)1) || (ctcpCount == 2 && parsedLine[3][0] == (char)1 && parsedLine[3][parsedLine[3].size() - 1] == (char)1)) {
			std::string ctcpMsg = parsedLine[3].substr(1);
			if (parsedLine[3][parsedLine[3].size() - 1] == (char)1)
				ctcpMsg = ctcpMsg.substr(0, ctcpMsg.size() - 1);
			size_t ctcpSpace = ctcpMsg.find(' ');
			std::string ctcp = ctcpMsg.substr(0, ctcpSpace);
			std::string params;
			if (ctcpSpace != std::string::npos)
				params = ctcpMsg.substr(ctcpSpace + 1);
			// Call the appropriate hook based on whether it's being sent to a channel or a user
			if (chanTypes.find(parsedLine[2][0]) != chanTypes.end())
				callChanCTCPHook(client, parsedLine[2], ' ', nick, ctcp, params);
			else if (chanTypes.find(parsedLine[2][1]) != chanTypes.end()) {
				bool chanCTCP = false;
				for (std::pair<std::string, char> status : prefixes) {
					if (status.second == parsedLine[2][0]) {
						chanCTCP = true;
						callChanCTCPHook(client, parsedLine[2].substr(1), parsedLine[2][0], nick, ctcp, params);
						break;
					}
				}
				if (!chanCTCP)
					callUserCTCPHook(client, nick, ctcp, params);
			} else
				callUserCTCPHook(client, nick, ctcp, params);
		} else {
			// Either none or part of the message is CTCP; pick out the CTCP parts to make appropriate hook calls
			std::list<std::pair<std::string, std::string>> ctcpBits;
			size_t ctcpPos = 0;
			ctcpPos = parsedLine[3].find((char)1);
			while (ctcpPos != std::string::npos) {
				size_t startCTCP = ctcpPos;
				ctcpPos = parsedLine[3].find((char)1, ctcpPos + 1);
				std::string ctcpSegment = parsedLine[3].substr(startCTCP + 1, ctcpPos - (startCTCP + 1));
				if (!ctcpSegment.empty()) {
					size_t spacePos = ctcpSegment.find(' ');
					if (spacePos == std::string::npos)
						ctcpBits.push_back(std::pair<std::string, std::string> (ctcpSegment, ""));
					else
						ctcpBits.push_back(std::pair<std::string, std::string> (ctcpSegment.substr(0, spacePos), ctcpSegment.substr(spacePos + 1)));
				}
				ctcpPos = parsedLine[3].find((char)1, ctcpPos + 1);
			}
			// Call the appropriate hooks for the whole message and the CTCP bits based on whether it's being sent to a channel or to a user
			if (chanTypes.find(parsedLine[2][0]) != chanTypes.end()) {
				callChanMsgHook(client, parsedLine[2], ' ', nick, parsedLine[3]);
				for (std::pair<std::string, std::string> ctcp : ctcpBits)
					callChanCTCPHook(client, parsedLine[2], ' ', nick, ctcp.first, ctcp.second);
			} else if (chanTypes.find(parsedLine[2][1]) != chanTypes.end()) {
				bool chanCTCP = false;
				for (std::pair<std::string, char> status : prefixes) {
					if (status.second == parsedLine[2][0]) {
						chanCTCP = true;
						callChanMsgHook(client, parsedLine[2].substr(1), parsedLine[2][0], nick, parsedLine[3]);
						for (std::pair<std::string, std::string> ctcp : ctcpBits)
							callChanCTCPHook(client, parsedLine[2].substr(1), parsedLine[2][0], nick, ctcp.first, ctcp.second);
						break;
					}
				}
				if (!chanCTCP) {
					callUserMsgHook(client, nick, parsedLine[3]);
					for (std::pair<std::string, std::string> ctcp : ctcpBits)
						callUserCTCPHook(client, nick, ctcp.first, ctcp.second);
				}
			} else {
				callUserMsgHook(client, nick, parsedLine[3]);
				for (std::pair<std::string, std::string> ctcp : ctcpBits)
					callUserCTCPHook(client, nick, ctcp.first, ctcp.second);
			}
		}
	} else if (parsedLine[1] == "NOTICE") {
		std::string hostmask = parsedLine[0];
		if (hostmask[0] == ':')
			hostmask = hostmask.substr(1);
		std::string nick;
		std::tie(nick, std::ignore, std::ignore) = parseHostmask(hostmask);
		size_t ctcpCount = 0;
		for (char msgChar : parsedLine[3]) {
			if (msgChar == (char)1)
				ctcpCount++;
		}
		// Detect whether the whole message is a CTCP
		if ((ctcpCount == 1 && parsedLine[3][0] == (char)1) || (ctcpCount == 2 && parsedLine[3][0] == (char)1 && parsedLine[3][parsedLine[3].size() - 1] == (char)1)) {
			std::string ctcpMsg = parsedLine[3].substr(1);
			if (parsedLine[3][parsedLine[3].size() - 1] == (char)1)
				ctcpMsg = ctcpMsg.substr(0, ctcpMsg.size() - 1);
			size_t ctcpSpace = ctcpMsg.find(' ');
			std::string ctcp = ctcpMsg.substr(0, ctcpSpace);
			std::string params;
			if (ctcpSpace != std::string::npos)
				params = ctcpMsg.substr(ctcpSpace + 1);
			// Call the appropriate hook based on whether it's being sent to a channel or a user
			if (chanTypes.find(parsedLine[2][0]) != chanTypes.end())
				callChanCTCPReplyHook(client, parsedLine[2], ' ', nick, ctcp, params);
			else if (chanTypes.find(parsedLine[2][1]) != chanTypes.end()) {
				bool chanCTCP = false;
				for (std::pair<std::string, char> status : prefixes) {
					if (status.second == parsedLine[2][0]) {
						chanCTCP = true;
						callChanCTCPReplyHook(client, parsedLine[2].substr(1), parsedLine[2][0], nick, ctcp, params);
						break;
					}
				}
				if (!chanCTCP)
					callUserCTCPReplyHook(client, nick, ctcp, params);
			} else
				callUserCTCPReplyHook(client, nick, ctcp, params);
		} else {
			// Either none or part of the message is CTCP; pick out the CTCP parts to make appropriate hook calls
			std::list<std::pair<std::string, std::string>> ctcpBits;
			size_t ctcpPos = 0;
			ctcpPos = parsedLine[3].find((char)1);
			while (ctcpPos != std::string::npos) {
				size_t startCTCP = ctcpPos;
				ctcpPos = parsedLine[3].find((char)1, ctcpPos + 1);
				std::string ctcpSegment = parsedLine[3].substr(startCTCP + 1, ctcpPos - (startCTCP + 1));
				if (!ctcpSegment.empty()) {
					size_t spacePos = ctcpSegment.find(' ');
					if (spacePos == std::string::npos)
						ctcpBits.push_back(std::pair<std::string, std::string> (ctcpSegment, ""));
					else
						ctcpBits.push_back(std::pair<std::string, std::string> (ctcpSegment.substr(0, spacePos), ctcpSegment.substr(spacePos + 1)));
				}
				ctcpPos = parsedLine[3].find((char)1, ctcpPos + 1);
			}
			// Call the appropriate hooks for the whole message and the CTCP bits based on whether it's being sent to a channel or to a user
			if (chanTypes.find(parsedLine[2][0]) != chanTypes.end()) {
				callChanNoticeHook(client, parsedLine[2], ' ', nick, parsedLine[3]);
				for (std::pair<std::string, std::string> ctcp : ctcpBits)
					callChanCTCPReplyHook(client, parsedLine[2], ' ', nick, ctcp.first, ctcp.second);
			} else if (chanTypes.find(parsedLine[2][1]) != chanTypes.end()) {
				bool chanCTCP = false;
				for (std::pair<std::string, char> status : prefixes) {
					if (status.second == parsedLine[2][0]) {
						chanCTCP = true;
						callChanNoticeHook(client, parsedLine[2].substr(1), parsedLine[2][0], nick, parsedLine[3]);
						for (std::pair<std::string, std::string> ctcp : ctcpBits)
							callChanCTCPReplyHook(client, parsedLine[2].substr(1), parsedLine[2][0], nick, ctcp.first, ctcp.second);
						break;
					}
				}
				if (!chanCTCP) {
					callUserNoticeHook(client, nick, parsedLine[3]);
					for (std::pair<std::string, std::string> ctcp : ctcpBits)
						callUserCTCPReplyHook(client, nick, ctcp.first, ctcp.second);
				}
			} else {
				callUserNoticeHook(client, nick, parsedLine[3]);
				for (std::pair<std::string, std::string> ctcp : ctcpBits)
					callUserCTCPReplyHook(client, nick, ctcp.first, ctcp.second);
			}
		}
	} else if (parsedLine[1] == "MODE") {
		std::string sourceHostmask (parsedLine[0]);
		if (sourceHostmask[0] == ':')
			sourceHostmask = sourceHostmask.substr(1);
		std::string nick, ident, host;
		std::tie(nick, ident, host) = parseHostmask(sourceHostmask);
		if (nick == clientIter->second->nick) {
			// Update the ident and the host of this client so that we're always fairly up-to-date even in the case of our ident/host being changed by opers or whatever.
			clientIter->second->ident = ident;
			clientIter->second->host = host;
		}
		if (parsedLine[2] == clientIter->second->nick) {
			bool adding = true;
			for (char mode : parsedLine[3]) {
				if (mode == '+')
					adding = true;
				else if (mode == '-')
					adding = false;
				else {
					std::string longMode;
					std::unordered_map<char, std::string>::iterator convIter = convertUserMode.find(mode);
					if (convIter == convertUserMode.end())
						longMode = mode;
					else
						longMode = convIter->second;
					if (longMode == "snomask") {
						bool snoAdding = true;
						for (char snomask : parsedLine[4]) {
							if (snomask == '+')
								adding = true;
							else if (snomask == '-')
								adding = false;
							else if (adding && clientIter->second->snomasks.find(snomask) == clientIter->second->snomasks.end()) {
								clientIter->second->snomasks.insert(snomask);
								callUserSNOmaskHook(clientIter->second->nick, true, snomask);
							} else if (!adding && clientIter->second->snomasks.find(snomask) != clientIter->second->snomasks.end()) {
								clientIter->second->snomasks.erase(snomask);
								callUserSNOmaskHook(clientIter->second->nick, false, snomask);
							}
						}
					} else {
						if (adding && clientIter->second->modes.find(longMode) == clientIter->second->modes.end()) {
							clientIter->second->modes.insert(longMode);
							callUserModeHook(clientIter->second->nick, true, longMode);
						} else if (!adding && clientIter->second->modes.find(longMode) != clientIter->second->modes.end()) {
							clientIter->second->modes.erase(longMode);
							callUserModeHook(clientIter->second->nick, false, longMode);
						}
					}
				}
			}
		} else { // channel
			std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(parsedLine[2]);
			size_t currParam = 4;
			bool adding = true;
			for (char mode : parsedLine[3]) {
				if (mode == '+')
					adding = true;
				else if (mode == '-')
					adding = false;
				else {
					std::string longMode;
					std::unordered_map<char, std::string>::iterator convIter = convertChanMode.find(mode);
					if (convIter == convertChanMode.end())
						longMode = mode;
					else
						longMode = convIter->second;
					if (paramParamModes.find(longMode) != paramParamModes.end() || paramModes.find(longMode) != paramModes.end()) {
						std::list<std::string>::iterator modeIter = chanIter->second->modes.begin();
						for (; modeIter != chanIter->second->modes.end(); ++modeIter) {
							std::string chanMode (*modeIter);
							chanMode = chanMode.substr(0, chanMode.find('='));
							if (chanMode == longMode)
								break;
						}
						if (adding && modeIter != chanIter->second->modes.end() && *modeIter != (longMode + "=" + parsedLine[currParam])) {
							std::string param (parsedLine[currParam++]);
							chanIter->second->modes.remove(modeIter);
							chanIter->second->modes.insert(longMode + "=" + param);
							callChanModeHook(chanIter->first, true, longMode + "=" + param);
						} else if (adding && modeIter == chanIter->second->modes.end()) {
							chanIter->second->modes.insert(longMode + "=" + parsedLine[currParam++]);
							callChanModeHook(chanIter->first, true, longMode + "=" + param);
						} else if (!adding && modeIter != chanIter->second->modes.end()) {
							std::string param ((*modeIter).substr((*modeIter).find_first_of('=') + 1));
							chanIter->second->modes.remove(modeIter);
							callChanModeHook(chanIter->first, false, longMode + "=" + param);
						}
						if (!adding && paramParamModes.find(longMode) != paramParamModes.end())
							currParam++;
					} else if (listModes.find(longMode) != listModes.end()) {
						std::unordered_map<std::string, std::list<std::string>>::iterator listIter = chanIter->second->listModes[longMode].begin();
						for (; listIter != chanIter->second->listModes[longMode].end(); ++listIter) {
							if (listIter == parsedLine[currParam])
								break;
						}
						if (adding && listIter == chanIter->second->listModes[longMode].end()) {
							std::string param (parsedLine[currParam++]);
							chanIter->second->listModes[longMode].push_back(param);
							callChanModeHook(chanIter->first, true, longMode + "=" + param);
						} else if (!adding && listIter != chanIter->second->listModes[longMode].end()) {
							chanIter->second->listModes[longMode].erase(listIter);
							callChanModeHook(chanIter->first, false, longMode + "=" + parsedLine[currParam++]);
						}
					} else if (normalModes.find(longMode) != normalModes.end()) {
						std::unordered_map<std::string, std::list<std::string>>::iterator modeIter = chanIter->second->modes.begin();
						for (; modeIter != chanIter->second->modes.end(); ++modeIter) {
							if (*modeIter == longMode)
								break;
						}
						if (adding && modeIter == chanIter->second->modes.end()) {
							chanIter->second->modes.push_back(longMode);
							callChanModeHook(chanIter->first, true, longMode);
						} else if (!adding && modeIter != chanIter->second->modes.end()) {
							chanIter->second->modes.erase(modeIter);
							callChanModeHook(chanIter->first, false, longMode);
						}
					} else { // status
						char symbol = ' ';
						for (std::pair<std::string, char> status : prefixes) {
							if (status.first == longMode) {
								symbol = status.second;
								break;
							}
						}
						if (symbol != ' ') {
							std::list<char>::iterator statusIter = chanIter->second->statuses[params[currParam]].begin();
							for (; statusIter != chanIter->second->statuses[params[currParam]].end(); ++statusIter) {
								if (*statusIter == symbol)
									break;
							}
							if (adding && statusIter == chanIter->second->statuses[params[currParam]].end()) {
								std::list<std::pair<std::string, char>>::iterator prefixIter = prefixes.begin();
								bool success = false;
								for (std::list<char>::iterator userStatusIter = chanIter->second->statuses[params[currParam]].begin(); userStatusIter != chanIter->second->statuses[params[currParam]].end(); ++userStatusIter) {
									while (prefixIter != prefixes.end() && prefixIter->second != *userStatusIter) {
										if (prefixIter->second == symbol) {
											chanIter->second->statuses[params[currParam]].insert(userStatusIter, symbol);
											callChanStatusHook(chanIter->first, true, params[currParam], prefixIter->first);
											success = true;
											break;
										}
										++prefixIter;
									}
									if (success)
										break;
								}
								if (!success && prefixIter == prefixes.end()) {
									chanIter->second->statuses[params[currParam]].push_back(symbol);
									callChanStatusHook(chanIter->first, true, parsedLine[currParam], longMode);
								}
							} else if (!adding && statusIter != chanIter->second->statuses[params[currParam]].end()) {
								chanIter->second->statuses[params[currParam]].erase(statusIter);
								callChanStatusHook(chanIter->first, false, params[currParam], longMode);
							}
							currParam++;
						}
					}
				}
			}
		}
	} else if (parsedLine[1] == "JOIN") {
		std::string sourceHostmask (parsedLine[0]);
		if (sourceHostmask[0] == ':')
			sourceHostmask = sourceHostmask.substr(1);
		std::string nick, ident, host;
		std::tie(nick, ident, host) = parseHostmask(sourceHostmask);
		if (nick == clientIter->second->nick) {
			// Update the ident and the host of this client so that we're always fairly up-to-date even in the case of our ident/host being changed by opers or whatever.
			clientIter->second->ident = ident;
			clientIter->second->host = host;
			// Also add the channel in question to the client's channels
			std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(parsedLine[2]);
			if (chanIter == channels.end())
				channels.insert(std::pair<std::string, std::shared_ptr<Channel>> (parsedLine[2], new Channel));
			clientIter->second->channels.insert(parsedLine[2]);
			callChanJoinHook(parsedLine[2], nick);
		} else {
			std::unordered_map<std::string, std::shared_ptr<User>>::iterator userIter = users.find(nick);
			if (userIter == users.end())
				userIter = users.insert(std::pair<std::string, std::shared_ptr<User>> (nick, new User (nick, ident, host))).first;
			userIter->second->channels.insert(parsedLine[1]);
			std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(parsedLine[2]);
			if (chanIter->second->users.find(nick) == chanIter->second.users.end()) {
				chanIter->second->users.insert(nick);
				callChanJoinHook(chanIter->first, nick);
			}
		}
	} else if (parsedLine[1] == "PART") {
		std::string sourceHostmask (parsedLine[0]);
		if (sourceHostmask[0] == ':')
			sourceHostmask = sourceHostmask.substr(1);
		std::string nick, ident, host;
		std::tie(nick, ident, host) = parseHostmask(sourceHostmask);
		if (nick == clientIter->second->nick) {
			clientIter->second->channels.erase(parsedLine[2]);
			bool inChannel = false;
			for (std::pair<std::string, std::shared_ptr<LocalClient>> client : connClients) {
				if (client.second->channels.find(parsedLine[2]) != client.second->channels.end()) {
					inChannel = true;
					break;
				}
			}
			callChanPartHook(parsedLine[2], nick, parsedLine[3]);
			if (!inChannel)
				channels.erase(channels.find(parsedLine[2]));
			else
				channels.find(parsedLine[2])->second->users.erase(nick);
		} else {
			std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(parsedLine[2]);
			std::set<std::string>::iterator usersIter = chanIter->second->users.find(nick);
			if (usersIter != chanIter->second->users.end()) {
				callChanPartHook(parsedLine[2], nick, parsedLine[3]);
				chanIter->second->users.erase(usersIter);
			}
			std::unordered_map<std::string, std::shared_ptr<User>>::iterator userIter = users.find(nick);
			if (userIter != users.end()) {
				userIter->second->channels.erase(parsedLine[2]);
				if (userIter->second->channels.empty())
					users.erase(userIter);
			}
		}
	} else if (parsedLine[1] == "QUIT") {
		std::string sourceHostmask (parsedLine[0]);
		if (sourceHostmask[0] == ':')
			sourceHostmask = sourceHostmask.substr(1);
		std::string nick;
		std::tie(nick, std::ignore, std::ignore) = parseHostmask(sourceHostmask);
		std::unordered_map<std::string, std::shared_ptr<User>>::iterator userIter = users.find(nick);
		if (userIter != users.end()) {
			callUserQuitHook(nick, parsedLine[2]);
			for (std::string chanName : userIter->second->channels)
				channels.find(chanName)->second->users.erase(nick);
			users.erase(userIter);
		}
	} else if (parsedLine[1] == "KICK") {
		if (parsedLine[3] == clientIter->second->nick) {
			clientIter->second->channels.erase(parsedLine[2]);
			bool clientInChannel = false;
			for (std::pair<std::string, std::shared_ptr<LocalClient>> client : connClients) {
				if (client.second->channels.find(parsedLine[2]) != client.second->channels.end()) {
					clientInChannel = true;
					break;
				}
			}
			std::string nick, sourceHostmask (parsedLine[0]);
			if (sourceHostmask[0] == ':')
				sourceHostmask = sourceHostmask.substr(1);
			std::tie(nick, std::ignore, std::ignore) = parseHostmask(sourceHostmask);
			callChanKickHook(parsedLine[2], nick, parsedLine[3], parsedLine[4]);
			if (clientInChannel)
				channels.find(parsedLine[2])->second->users.erase(parsedLine[3]);
			else
				channels.erase(channels.find(parsedLine[2]));
		} else {
			std::unordered_map<std::stirng, std::shared_ptr<Channel>>::iterator chanIter = channels.find(parsedLine[2]);
			std::set<std::string>::iterator usersIter = chanIter->second->users.find(parsedLine[3]);
			if (usersIter != chanIter->second->users.end()) {
				std::string nick, sourceHostmask (parsedLine[0]);
				if (sourceHostmask[0] == ':')
					sourceHostmask = sourceHostmask.substr(1);
				std::tie(nick, std::ignore, std::ignore) = parseHostmask(sourceHostmask);
				callChanKickHook(parsedLine[2], nick, parsedLine[3], parsedLine[4]);
				chanIter->second->users.erase(usersIter);
			}
			std::unordered_map<std::string, std::shared_ptr<User>>::iterator userIter = users.find(parsedLine[3]);
			if (userIter != users.end()) {
				userIter->second->channels.erase(parsedLine[2]);
				if (userIter->second->channels.empty())
					users.erase(userIter);
			}
		}
	} else if (parsedLine[1] == "TOPIC") {
		std::string setter, sourceHostmask (parsedLine[0]);
		if (sourceHostmask[0] == ':')
			sourceHostmask = sourceHostmask.substr(1);
		std::tie(setter, std::ignore, std::ignore) = parseHostmask(sourceHostmask);
		std::unordered_map<std::string, std::shared_ptr<Channel>>::iterator chanIter = channels.find(parsedLine[2]);
		chanIter->second->topic = parsedLine[3];
		chanIter->second->topicSetter = setter;
		callChanTopicHook(parsedLine[2], setter, parsedLine[3]);
	} else if (parsedLine[1] == "INVITE") {
		std::string inviter, sourceHostmask (parsedLine[0]);
		if (sourceHostmask[0] == ':')
			sourceHostmask = sourceHostmask.substr(1);
		std::tie(inviter, std::ignore, std::ignore) = parseHostmask(sourceHostmask);
		callChanInviteHook(parsedLine[3], inviter, parsedLine[2]);
	} else if (parsedLine[1] == "NICK") {
		std::string nick, ident, host, sourceHostmask (parsedLine[0]);
		if (sourceHostmask[0] == ':')
			sourceHostmask = sourceHostmask.substr(1);
		std::tie(nick, ident, host) = parseHostmask(sourceHostmask);
		std::unordered_map<std::string, std::shared_ptr<User>>::iterator userIter = users.find(nick);
		if (userIter == users.end()) {
			for (std::pair<std::string, std::shared_ptr<LocalClient>> client : connClients) {
				if (client.second->nick == nick) {
					client.second->nick = parsedLine[2];
					// May as well resync the ident and host while we're updating things, eh?
					client.second->ident = ident;
					client.second->host = host;
					callUserNickHook(nick, client.second->nick);
					break;
				}
			}
		} else {
			std::shared_ptr<User> userPtr = userIter->second;
			users.erase(userIter);
			userPtr->nick = parsedLine[2];
			users.insert(std::pair<std::string, std::shared_ptr<User>> (parsedLine[2], userPtr));
			callUserNickHook(nick, userPtr->nick);
		}
	} else
		callOtherDataHook(client, parsedLine);
}

std::vector<std::string> Client::parseLine(const std::string& line) {
	std::vector<std::string> tokens;
	std::string part;
	bool lastToken = false;
	for (char currChar : line) {
		if (lastToken) {
			part += currChar;
			continue;
		}
		if (currChar == ' ') {
			tokens.push_back(part);
			part.clear();
		} else if (currChar == ':' && part.empty() && !tokens.empty()) // If the colon is the first character in the word, but not the first character in the line
			lastToken = true;
		else
			part += currChar;
	}
	if (!part.empty())
		tokens.push_back(part);
	return tokens;
}

std::tuple<std::string, std::string, std::string> Client::parseHostmask(const std::string& hostmask) {
	std::string nick (hostmask);
	size_t identStart = nick.find('!');
	if (identStart == std::string::npos)
		return std::make_tuple(nick, "", "");
	std::string ident (nick.substr(identStart + 1));
	nick = nick.substr(0, identStart);
	size_t hostStart = ident.find('@');
	std::string host (ident.substr(hostStart + 1));
	ident = ident.substr(0, hostStart);
	return std::make_tuple(nick, ident, host);
}

void Client::registerLongMode(const std::string& modeName, char modeChar, bool chanMode) {
	if (!config["mode/" + modeName].empty()) {
		if (config["mode/" + modeName] == "disable" || config["mode/" + modeName] == "disabled")
			return;
		modeChar = config["mode/" + modeName][0];
	}
	convertMode.insert(std::pair<std::string, char> (modeName, modeChar));
	if (chanMode)
		convertChanMode.insert(std::pair<char, std::string> (modeChar, modeName));
	else
		convertUserMode.insert(std::pair<char, std::string> (modeChar, modeName));
}

std::string Client::newID() {
	std::ostringstream strID;
	strID << currID++;
	while (connClients.find(strID.str()) != connClients.end()) {
		strID.str("");
		strID << currID++;
	}
	return strID.str();
}

PROTOCOL_SPAWN(Client)