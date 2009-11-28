#include "main.h"
#include "socket.cpp"

#include <sstream>

#ifndef CONNECTION_DEF_ROBOBO
#define CONNECTION_DEF_ROBOBO
class Server;
class Channel;

class User {
	public:
		User(Channel* thisChannel, std::string theIdent, std::string theHost, std::string theGECOS);
		std::string getIdent();
		std::string getHost();
		std::string getName();
		void status(bool add, char status);
		Channel* parentChannel;
	private:
		std::string ident, host, gecos;
		std::tr1::unordered_map<char, bool> hasStatus;
};

class Channel {
	public:
		Channel(Server* thisServer);
		void parseNames(std::string numeric353);
		void clearUserlist();
		void setTopic(std::string newTopic);
		void setMode(bool add, char mode, std::string param = "");
		void joinChannel(std::string nick, std::string ident, std::string host, std::string gecos);
		void leaveChannel(std::string nick);
		Server* parentServer;
	private:
		std::tr1::unordered_map<std::string, User> users;
		std::string topic;
		std::tr1::unordered_map<char, std::string> modes; // does not include lists or statuses
		std::vector<std::string> banList, exceptList, invexList;
};

class Server {
	public:
		Server(std::string serverAddress, std::tr1::unordered_map<std::string, std::string> confVars, std::list<std::string> modList);
		void joinChannel(std::string channelName);
		// more interface functions soon
		std::tr1::unordered_map<char, char> getPrefixes(); // necessary in channels
		std::vector<std::vector<char> > getChanModes();
	private:
		Socket serverConnection;
		std::list<std::string> moduleList;
		std::tr1::unordered_map<std::string, std::string> serverConf;
		std::tr1::unordered_map<std::string, Channel> inChannels;
		std::string network;
		std::tr1::unordered_map<char, char> prefix;
		std::vector<char> chanTypes, statusMsg;
		std::vector<std::vector<char> > chanModes;
		void handleData();
		void parse005(std::vector<std::string> parsedLine);
		std::vector<std::string> parseLine(std::string unformattedLine);
		std::vector<std::string> separateBySpace(std::string joinedLine);
		std::string separateNickFromFullHostmask(std::string hostmask);
};
#endif