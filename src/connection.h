#ifndef CONNECTION_H
#define CONNECTION_H
#include "main.h"
#include "socket.h"

#include <map>
#include <sstream>
#include <queue>
#include <time.h>

// Forward declarations
class Server;
class Channel;
class ModuleInterface;

class User {
	public:
		User(Channel* thisChannel);
		void status(bool add, char status);
		char getStatus();
		Channel* parentChannel;
	private:
		std::map<char, bool> hasStatus;
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
		std::list<std::string> getUsers();
		char getStatus(std::string user);
		std::string getTopic();
		void changeNick(std::string oldNick, std::string newNick);
		Server* parentServer;
	private:
		std::tr1::unordered_map<std::string, User*> users;
		std::string topic;
		bool namesSync;
};

class Server {
	public:
		Server(std::string serverAddress, std::tr1::unordered_map<std::string, std::string> confVars, ModuleInterface* modFace, unsigned short debug);
		~Server();
		bool stillConnected();
		void sendLine(std::string line);
		std::tr1::unordered_map<std::string, std::string> getInfo();
		std::tr1::unordered_map<char, char> getPrefixes();
		std::vector<std::vector<char> > getChanModes();
		std::vector<char> getChanTypes();
		void resyncChannels();
		std::list<std::string> getChannels();
		std::string getChannelTopic(std::string channel);
		std::list<std::string> getChannelUsers(std::string channel);
		std::pair<char, char> getUserStatus(std::string channel, std::string user);
	private:
		std::string serverName;
		unsigned short debugLevel;
		bool registered, altChanged;
		Socket serverConnection;
		ModuleInterface* moduleData;
		pthread_t dataReceiveThread;
		pthread_t dataSendThread;
		pthread_t secondDecrementThread;
		pthread_mutex_t secondsmutex;
		pthread_attr_t detachedState;
		volatile unsigned short seconds; // seconds in the penalty system
		std::tr1::unordered_map<std::string, std::string> serverConf;
		std::tr1::unordered_map<std::string, Channel*> inChannels;
		std::string network;
		unsigned short maxModes;
		std::vector<char> userModes;
		std::tr1::unordered_map<char, char> prefix;
		std::vector<char> chanTypes;
		std::vector<std::vector<char> > chanModes;
		std::queue<std::string> outData;
		static void* handleData_thread(void* ptr);
		void handleData();
		static void* sendData_thread(void* ptr);
		void sendData();
		static void* secondDecrement_thread(void* ptr);
		void secondDecrement();
		void parse005(std::vector<std::string> parsedLine);
		std::vector<std::string> parseLine(std::string unformattedLine);
		std::vector<std::string> separateBySpace(std::string joinedLine);
		std::string separateNickFromFullHostmask(std::string hostmask);
};
#endif