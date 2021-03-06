#pragma once
#include "common.h"

class Protocol;

class Channel {
	public:
		Channel(std::string&& name, Protocol* mod);
		std::string name() const;
		time_t time() const;
		void time(time_t time);
		std::list<std::string> users() const;
		bool userInChan(const std::string& user) const;
		std::list<std::string> statuses(const std::string& user) const;
		bool userHasStatus(const std::string& user, const std::string& status) const;
		bool userHasStatusOrGreater(const std::string& user, const std::string& status) const;
		void addUser(const std::string& user);
		void addUser(const std::string& user, const std::list<std::string>& statuses);
		void removeUser(const std::string& user);
		void grantStatus(const std::string& user, const std::string& status);
		void revokeStatus(const std::string& user, const std::string& status);
		void clearUsers();
		bool usersSynced() const;
		std::string userSyncingClient() const;
		void setUserSyncingClient(const std::string& clientID);
		void setUsersSynced();
		bool isAdditionalSyncingClient(const std::string& clientID) const;
		void addAdditionalSyncingClient(const std::string& clientID);
		void removeAdditionalSyncingClient(const std::string& clientID);
		std::string topic() const;
		std::string topicSetter() const;
		time_t topicTime() const;
		void topic(const std::string& topic, const std::string& setter);
		void topicTime(time_t time);
		std::map<std::string, std::string> modes() const;
		bool modeSet(const std::string& mode) const;
		std::string modeParam(const std::string& mode) const;
		std::list<std::string> modeList(const std::string& mode) const;
		bool itemInList(const std::string& mode, const std::string& param) const;
		void setMode(const std::string& mode);
		void setMode(const std::string& mode, const std::string& param);
		void unsetMode(const std::string& mode);
		void unsetMode(const std::string& mode, const std::string& param);
		void clearModes();
		void clearListMode(const std::string& mode);
	private:
		const std::string chanName;
		time_t chanTimestamp;
		std::map<std::string, std::list<std::string>> chanUsers;
		std::string chanUsersSyncingClient;
		std::set<std::string> additionalSyncingClients;
		std::string chanTopic;
		std::string chanTopicSetter;
		time_t chanTopicTime;
		std::map<std::string, std::string> chanModes;
		std::map<std::string, std::list<std::string>> chanListModes;
		Protocol* const proto;
};

#include "protocol.h"