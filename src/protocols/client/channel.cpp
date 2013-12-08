#include "channel.h"

Channel::Channel(std::string&& name, Protocol* mod) : chanName(std::forward<std::string> (name)), proto(mod) {}

std::string Channel::name() const {
	return chanName;
}

time_t Channel::time() const {
	return chanTimestamp;
}

void Channel::time(time_t time) {
	chanTimestamp = time;
}

std::list<std::string> Channel::users() const {
	std::list<std::string> userList;
	for (auto user : chanUsers)
		userList.push_back(user.first);
	return userList;
}

bool Channel::userInChan(const std::string& user) const {
	return chanUsers.find(user) != chanUsers.end();
}

std::list<std::string> Channel::statuses(const std::string& user) const {
	auto userIter = chanUsers.find(user);
	if (userIter == chanUsers.end())
		return std::list<std::string> ();
	return userIter->second;
}

bool Channel::userHasStatus(const std::string& user, const std::string& status) const {
	auto userIter = chanUsers.find(user);
	if (userIter == chanUsers.end())
		return false;
	auto statusIter = std::find(userIter->second.begin(), userIter->second.end(), status);
	return statusIter != userIter->second.end();
}

bool Channel::userHasStatusOrGreater(const std::string& user, const std::string& status) const {
	auto userIter = chanUsers.find(user);
	if (userIter == chanUsers.end())
		return false;
	std::string userStatus (userIter->second.front());
	return proto->compareStatus(userStatus, status).first == userStatus;
}

void Channel::addUser(const std::string& user) {
	chanUsers.insert(std::pair<std::string, std::list<std::string>> (user, std::list<std::string> ()));
}

void Channel::addUser(const std::string& user, const std::list<std::string>& statuses) {
	chanUsers.insert(std::pair<std::string, std::list<std::string>> (user, statuses));
}

void Channel::removeUser(const std::string& user) {
	chanUsers.erase(user);
}

void Channel::grantStatus(const std::string& user, const std::string& status) {
	auto userIter = chanUsers.find(user);
	if (userIter == chanUsers.end())
		return;
	bool inserted = false;
	for (auto statusIter = userIter->second.cbegin(); statusIter != userIter->second.cend(); ++statusIter) {
		if (proto->compareStatus(status, *statusIter).first == status) {
			userIter->second.insert(statusIter, status);
			inserted = true;
			break;
		}
	}
	if (!inserted)
		userIter->second.push_back(status);
}

void Channel::revokeStatus(const std::string& user, const std::string& status) {
	auto userIter = chanUsers.find(user);
	if (userIter == chanUsers.end())
		return;
	userIter->second.remove(status);
}

void Channel::clearUsers() {
	chanUsers.clear();
}

std::string Channel::topic() const {
	return chanTopic;
}

std::string Channel::topicSetter() const {
	return chanTopicSetter;
}

time_t Channel::topicTime() const {
	return chanTopicTime;
}

void Channel::topic(const std::string& topic, const std::string& setter) {
	chanTopic = topic;
	chanTopicSetter = setter;
	chanTopicTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

void Channel::topicTime(time_t time) {
	chanTopicTime = time;
}

std::map<std::string, std::string> Channel::modes() const {
	return chanModes;
}

bool Channel::modeSet(const std::string& mode) const {
	return chanModes.find(mode) != chanModes.end();
}

std::string Channel::modeParam(const std::string& mode) const {
	auto modeIter = chanModes.find(mode);
	if (modeIter == chanModes.end())
		return "";
	return modeIter->second;
}

std::list<std::string> Channel::modeList(const std::string& mode) const {
	auto listModeIter = chanListModes.find(mode);
	if (listModeIter == chanListModes.end())
		return std::list<std::string> ();
	return listModeIter->second;
}

bool Channel::itemInList(const std::string& mode, const std::string& param) const {
	auto listModeIter = chanListModes.find(mode);
	if (listModeIter == chanListModes.end())
		return false;
	auto listIter = std::find(listModeIter->second.begin(), listModeIter->second.end(), param);
	return listIter != listModeIter->second.end();
}

void Channel::setMode(const std::string& mode) {
	chanModes.insert(std::pair<std::string, std::string> (mode, ""));
}

void Channel::setMode(const std::string& mode, const std::string& param) {
	if (proto->chanModeType(mode) == MODE_LIST) {
		auto listModeIter = chanListModes.find(mode);
		if (listModeIter == chanListModes.end()) {
			chanListModes[mode].push_back(param);
			return;
		}
		auto listIter = std::find(listModeIter->second.begin(), listModeIter->second.end(), param);
		if (listIter == listModeIter->second.end())
			listModeIter->second.push_back(param);
	} else
		chanModes[mode] = param;
}

void Channel::unsetMode(const std::string& mode) {
	chanModes.erase(mode);
}

void Channel::unsetMode(const std::string& mode, const std::string& param) {
	if (proto->chanModeType(mode) == MODE_LIST) {
		auto listModeIter = chanListModes.find(mode);
		if (listModeIter == chanListModes.end())
			return;
		listModeIter->second.remove(param);
		if (listModeIter->second.empty())
			chanListModes.erase(listModeIter);
	} else
		unsetMode(mode);
}

void Channel::clearModes() {
	chanModes.clear();
}

void Channel::clearListMode(const std::string& mode) {
	auto modeIter = chanListModes.find(mode);
	if (modeIter != chanListModes.end())
		chanListModes.erase(modeIter);
}