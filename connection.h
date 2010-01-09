#include "main.h"
#include "socket.cpp"

#include <sstream>

#ifndef CONNECTION_H
#define CONNECTION_H
class Server;
class Channel;

class User {
	public:
		User(Channel* thisChannel);
		void status(bool add, char status);
		Channel* parentChannel;
	private:
		std::tr1::unordered_map<char, bool> hasStatus;
};

class Channel {
	public:
		Channel(Server* thisServer);
		void parseNames(std::vector<std::string> names);
		void numeric366();
		void setTopic(std::string newTopic);
		void setMode(bool add, char mode, std::string param = "");
		void joinChannel(std::string nick);
		void leaveChannel(std::string nick);
		Server* parentServer;
	private:
		std::tr1::unordered_map<std::string, User*> users;
		std::string topic;
		bool namesSync;
};

class Server {
	public:
		Server(std::string serverAddress, std::tr1::unordered_map<std::string, std::string> confVars, ModuleInterface* modFace);
		void sendLine(std::string line);
		std::tr1::unordered_map<std::string, std::string> getInfo();
		std::tr1::unordered_map<char, char> getPrefixes(); // necessary in channels
		std::vector<std::vector<char> > getChanModes();
		std::vector<char> getChanTypes();
		void resyncChannels();
	private:
		std::string serverName;
		bool registered;
		Socket serverConnection;
		ModuleInterface* moduleData;
		pthread_t dataReceiveThread;
		std::tr1::unordered_map<std::string, std::string> serverConf;
		std::tr1::unordered_map<std::string, Channel*> inChannels;
		std::string network;
		std::vector<char> userModes;
		std::tr1::unordered_map<char, char> prefix;
		std::vector<char> chanTypes;
		std::vector<std::vector<char> > chanModes;
		static void* handleData_thread(void* ptr);
		void handleData();
		void parse005(std::vector<std::string> parsedLine);
		std::vector<std::string> parseLine(std::string unformattedLine);
		std::vector<std::string> separateBySpace(std::string joinedLine);
		std::string separateNickFromFullHostmask(std::string hostmask);
};
#endif