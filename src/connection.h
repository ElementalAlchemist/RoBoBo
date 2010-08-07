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
		User(std::string theTheIdent, std::string theTheHost, Channel* thisChannel);
		void ident(std::string newIdent);
		std::string ident();
		void host(std::string newHost);
		std::string host();
		void status(bool add, char status);
		char status();
		Channel* parentChannel;
	private:
		std::string theIdent, theHost;
		std::vector<std::pair<char, bool> > hasStatus;
};

class Channel {
	public:
		Channel(Server* thisServer);
		void parseNames(std::vector<std::string> names);
		void numeric366();
		void topic(std::string newTopic);
		void mode(bool add, char mode, std::string param = "");
		void joinChannel(std::string hostmask);
		void leaveChannel(std::string nick);
		void nick(std::string oldNick, std::string newNick);
		std::list<std::string> users();
		void ident(std::string user, std::string ident);
		std::string ident(std::string user);
		void host(std::string user, std::string host);
		std::string host(std::string user);
		char status(std::string user);
		std::string topic();
		Server* parentServer;
	private:
		std::tr1::unordered_map<std::string, User*> theUsers;
		std::string theTopic;
		bool namesSync;
};

class Server {
	public:
		Server(std::string serverAddress, std::tr1::unordered_map<std::string, std::string> confVars, ModuleInterface* modFace, unsigned short debug);
		~Server();
		void connectServer();
		bool stillConnected();
		bool shouldReset();
		void sendLine(std::string line);
		std::tr1::unordered_map<std::string, std::string> getInfo();
		std::vector<std::pair<char, char> > getPrefixes();
		std::vector<std::vector<char> > getChanModes();
		std::vector<char> getChanTypes();
		void resyncChannels();
		std::list<std::string> getChannels();
		std::string getChannelTopic(std::string channel);
		std::list<std::string> getChannelUsers(std::string channel);
		std::string getUserIdent(std::string channel, std::string user);
		std::string getUserHost(std::string channel, std::string user);
		std::pair<char, char> getUserStatus(std::string channel, std::string user);
	private:
		std::string serverName;
		unsigned short debugLevel;
		bool registered, altChanged, quitHooked, keepServer;
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
		std::vector<std::pair<char, char> > prefix;
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