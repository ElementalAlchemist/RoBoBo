#pragma once
#include "common.h"
#include "user.h"
#include <sys/socket.h>

class Protocol;

class Client : public User {
	public:
		Client(std::string&& id, std::string&& nick, std::string&& ident, std::string&& gecos, Protocol* mod);
		~Client();
		void disconnect();
		bool checkConnection() const;
		bool wantsToReconnect() const;
		void doReconnect();
		std::map<std::string, std::string> modes() const;
		bool modeSet(const std::string& mode) const;
		std::string modeParam(const std::string& mode) const;
		std::list<std::string> listModeList(const std::string& mode) const;
		bool itemInList(const std::string& mode, const std::string& param) const;
		void setMode(const std::string& mode);
		void setMode(const std::string& mode, const std::string& param);
		void unsetMode(const std::string& mode);
		void setListMode(const std::string& mode, const std::string& param);
		void unsetListMode(const std::string& mode, const std::string& param);
		void sendLine(const IRCMessage* line);
	private:
		std::map<std::string, std::string> clientModes;
		std::map<std::string, std::list<std::string>> clientListModes;
		std::shared_ptr<Socket> socket;
		bool expectingReconnect;
		Protocol* proto;
		std::thread receiveThread;
		std::thread sendThread;
		std::thread secondsThread;
		std::mutex sendMutex;
		void receiveData();
		void sendQueue();
		void decrementSeconds();
};