#pragma once
#include "common.h"
#include "user.h"
#include "client.h"
#include "channel.h"

class Protocol : public ClientProtocol {
	public:
		Protocol();
		~Protocol();
		unsigned int apiVersion() { return 3000; }
		void connectServer();
		bool connected();
		bool shouldUnload();
		void disconnect(const std::string& reason);
		void onRehash();
		
		void sendMsg(const std::string& client, const std::string& target, const std::string& message, const std::map<std::string, std::string>& tags);
		void sendNotice(const std::string& client, const std::string& target, const std::string& message, const std::map<std::string, std::string>& tags);
		void setMode(const std::string& client, const std::string& target, std::list<std::tuple<bool, std::string, std::string>> modes, const std::map<std::string, std::string>& tags);
		void joinChan(const std::string& client, const std::string& channel, const std::map<std::string, std::string>& tags);
		void joinChan(const std::string& client, const std::string& channel, const std::string& key, const std::map<std::string, std::string>& tags);
		void partChan(const std::string& client, const std::string& channel, const std::string& reason, const std::map<std::string, std::string>& tags);
		void kickUser(const std::string& client, const std::string& channel, const std::string& user, const std::string& reason, const std::map<std::string, std::string>& tags);
		void setTopic(const std::string& client, const std::string& channel, const std::string& topic, const std::map<std::string, std::string>& tags);
		void inviteUser(const std::string& client, const std::string& channel, const std::string& user, const std::map<std::string, std::string>& tags);
		void knock(const std::string& client, const std::string& channel, const std::string& reason, const std::map<std::string, std::string>& tags);
		void changeNick(const std::string& client, const std::string& newNick, const std::map<std::string, std::string>& tags);
		void sendPing(const std::string& client, const std::string& data, const std::map<std::string, std::string>& tags);
		void setAway(const std::string& client, const std::string& reason, const std::map<std::string, std::string>& tags);
		void setUnaway(const std::string& client, const std::map<std::string, std::string>& tags);
		void oper(const std::string& client, const std::string& username, const std::string& password, const std::map<std::string, std::string>& tags);
		void sendWallops(const std::string& client, const std::string& message, const std::map<std::string, std::string>& tags);
		void sendOtherData(const std::string& client, const IRCMessage* line);
		
		std::string addClient(const std::string& nick, const std::string& ident, const std::string& gecos, const std::string& password, const std::string& socket = "");
		void removeClient(const std::string& client, const std::string& reason);
		
		std::set<std::string> serverCapabilities();
		
		std::set<char> chanTypes();
		std::list<std::pair<ModeType, std::string>> allChanModes();
		ModeType chanModeType(const std::string& mode);
		char prefixSymbol(const std::string& mode);
		std::pair<std::string, char> compareStatus(const std::string& status0, const std::string& status1);
		std::pair<std::string, char> compareStatus(const std::string& status0, char status1);
		std::pair<std::string, char> compareStatus(char status0, char status1);
		
		std::string chanTopic(const std::string& channel);
		std::string chanTopicSetter(const std::string& channel);
		time_t chanTopicTimestamp(const std::string& channel);
		time_t chanTimestamp(const std::string& channel);
		std::list<std::string> chanUsers(const std::string& channel);
		bool userInChan(const std::string& channel, const std::string& user);
		std::pair<std::string, char> userStatus(const std::string& channel, const std::string& user);
		bool userHasStatus(const std::string& channel, const std::string& user, const std::string& status);
		bool userHasStatus(const std::string& channel, const std::string& user, char status);
		bool userHasStatusOrGreater(const std::string& channel, const std::string& user, const std::string& status);
		bool userHasStatusOrGreater(const std::string& channel, const std::string& user, char status);
		std::map<std::string, std::string> chanModes(const std::string& channel);
		bool chanHasMode(const std::string& channel, const std::string& mode);
		std::string chanModeParam(const std::string& channel, const std::string& mode);
		std::list<std::string> chanListModeList(const std::string& channel, const std::string& mode);
		
		std::list<std::string> clientList();
		std::string userNick(const std::string& user);
		std::string userIdent(const std::string& user);
		std::string userHost(const std::string& user);
		std::string userGecos(const std::string& user);
		std::string idFromNick(const std::string& nick);
		std::list<std::pair<ModeType, std::string>> allUserModes();
		ModeType userModeType(const std::string& mode);
		std::map<std::string, std::string> userModes(const std::string& user);
		bool userHasMode(const std::string& user, const std::string& mode);
		std::string userModeParam(const std::string& user, const std::string& mode);
		std::list<std::string> userListModeList(const std::string& user, const std::string& listMode);
		std::set<std::string> userChans(const std::string& user);
		
		std::string servName();
		std::shared_ptr<Socket> obtainSocket(const std::string& sockType);
		void connectSocket(const std::shared_ptr<Socket> sock);
		bool floodThrottleInEffect();
		void processIncoming(const std::string& client, const IRCMessage* message);
	private:
		std::string serverAddress;
		std::string serverPort;
		std::string serverBindAddr;
		bool floodThrottle;
		bool loaded;
		std::mutex processMutex;
		unsigned int nextID;
		std::string getNextID();
		
		std::unordered_map<std::string, std::shared_ptr<User>> users;
		std::unordered_map<std::string, std::shared_ptr<Channel>> channels;
		std::unordered_map<std::string, std::shared_ptr<Client>> clients;
		std::unordered_map<std::string, std::string> nickToID;
		
		std::list<std::string> convertCommaSeparatedList(std::string str);
		std::string convertListToCommaSeparatedString(const std::list<std::string>& list);
		
		std::set<std::string> capabilities;
		std::set<char> channelTypes;
		
		unsigned int maxModes;
		unsigned int maxTargets;
		
		void loadModeNamesAndDefaults(std::unordered_map<std::string, std::string> modeConfig);
		void saveChanMode(const std::string& name, char letter, const std::string& override);
		void saveUserMode(const std::string& name, char letter, const std::string& override);
		
		std::unordered_map<std::string, char> chanModeStrToChar;
		std::unordered_map<char, std::string> chanModeCharToStr;
		std::unordered_map<std::string, char> userModeStrToChar;
		std::unordered_map<char, std::string> userModeCharToStr;
		
		std::unordered_multimap<ModeType, std::string> serverChanModes;
		std::unordered_multimap<ModeType, std::string> serverUserModes;
		std::unordered_map<std::string, ModeType> serverChanModeType;
		std::unordered_map<std::string, ModeType> serverUserModeType;
		
		std::unordered_map<std::string, char> chanPrefixModeToSymbol;
		std::unordered_map<char, std::string> chanPrefixSymbolToMode;
		std::list<std::string> chanPrefixOrder;
		
		void handleData();
		std::thread dataThread;
		std::queue<std::pair<std::string, std::unique_ptr<IRCMessage>>> receivedData;
};