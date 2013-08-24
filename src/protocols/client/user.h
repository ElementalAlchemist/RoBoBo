#pragma once
#include "common.h"

class User {
	public:
		User(std::string&& id, std::string&& nick, std::string&& ident, std::string&& gecos);
		virtual ~User();
		std::string id() const;
		std::string nick() const;
		void nick(const std::string& nick);
		std::string ident() const;
		void ident(const std::string& ident);
		std::string host() const;
		void host(const std::string& host);
		std::string gecos() const;
		void gecos(const std::string& gecos);
		bool away() const;
		std::string awayReason() const;
		void setAway(const std::string& reason);
		void setUnaway();
		std::set<std::string> channels() const;
		bool inChan(const std::string& channel) const;
		void addToChan(const std::string& channel);
		void removeFromChan(const std::string& channel);
	protected:
		const std::string userID;
		std::string userNick;
		std::string userIdent;
		std::string userHost;
		std::string userGecos;
		bool userAway;
		std::string userAwayReason;
		std::set<std::string> userChannels;
};