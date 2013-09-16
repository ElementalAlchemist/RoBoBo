#pragma once
#include "common.h"
#include "user.h"
#include <sys/socket.h>

class Protocol;

class Client : public User {
	public:
		Client(const std::string& id, std::string&& nick, std::string&& ident, std::string&& gecos, std::string&& pass, std::string&& socktype, Protocol* mod);
		~Client();
		void connect();
		void disconnect(const std::string& reason);
		bool checkConnection() const;
		bool wantsToReconnect() const;
		void doReconnect();
		void doRegister();
		void startFloodThrottle();
		void endFloodThrottle();
		std::map<std::string, std::string> modes() const;
		bool modeSet(const std::string& mode) const;
		std::string modeParam(const std::string& mode) const;
		std::list<std::string> modeList(const std::string& mode) const;
		bool itemInList(const std::string& mode, const std::string& param) const;
		void setMode(const std::string& mode);
		void setMode(const std::string& mode, const std::string& param);
		void unsetMode(const std::string& mode);
		void unsetMode(const std::string& mode, const std::string& param);
		void sendLine(const IRCMessage* line);
	private:
		std::string password;
		std::map<std::string, std::string> clientModes;
		std::map<std::string, std::list<std::string>> clientListModes;
		std::shared_ptr<Socket> socket;
		bool expectingReconnect;
		Protocol* const proto;
		std::thread receiveThread;
		std::thread sendThread;
		std::thread secondsThread;
		std::thread registerThread;
		std::mutex sendMutex;
		void receiveData();
		void sendQueue();
		void decrementSeconds();
		void delayRegister();
		bool needRegisterDelay;
		std::queue<std::unique_ptr<IRCMessage>> linesToSend;
		unsigned int penaltySeconds;
		std::map<std::string, unsigned int> commandPenalty;
};

#include "protocol.h"