#include "user.h"

#ifndef USER_ROBOBO
#define USER_ROBOBO
class User {
	public:
		User(std::string newNick);
		User(std::string newNick, std::vector<bool> levels);
		void addStatus(int statusIndex);
		void removeStatus(int statusIndex);
	private:
		void constructCommon();
		std::string nick;
		std::string ident;
		std::string host;
		std::string realname;
		std::vector<bool> level;
};

User::User(std::string newNick) {
	nick = newNick;
	constructCommon();
}

User::User(std::string newNick, std::vector<bool> levels) {
	nick = newNick;
	level = levels;
	constructCommon();
}

void User::constructCommon() {
	ident = "";
	host = "";
	realname = "";
}

void User::addStatus(int statusIndex) {
	level[statusIndex] = true;
}

void User::removeStatus(int statusIndex) {
	level[statusIndex] = false;
}
#endif