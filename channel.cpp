#include "channel.h"

#ifndef CHANNEL_ROBOBO
#define CHANNEL_ROBOBO
class Channel {
	public:
		Channel();
		void parseNames(std::string numeric353);
		void setTopic(std::string newTopic);
		void setMode(bool add, char mode, std::string param = "");
		void joinChannel(std::string nick, std::string ident, std::string host, std::string gecos);
		void partChannel(std::string nick);
		std::vector<std::string> banList();
	private:
		std::tr1::unordered_map<std::string, User> users;
		std::string topic;
		std::tr1::unordered_map<char, std::string> modes; // does not include lists or statuses
}

Channel::Channel() {
	topic = "";
}

void Channel::parseNames(std::string numeric353) {
	// will handle this soon
}

void Channel::setTopic(std::string newTopic) {
	topic = newTopic;
}

void Channel::setMode(bool add, char mode, std::string param = "") {
	// check whether it's a status or list mode before adding
}
#endif