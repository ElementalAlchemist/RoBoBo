#include "moduleBase.h"

class module {
	public:
		onChannelMsg(std::string channel, std::string nick, std::string message) = 0;
		onUserMsg(std::string nick, std::string message) = 0;
		onChannelNotice(std::string channel, char target, std::string nick, std::string message) = 0;
		onUserNotice(std::string nick, std::string message) = 0;
		onChannelCTCP(std::string channel, std::string nick, std::string type, std::string params) = 0;
		onUserCTCP(std::string nick, std::string type, std::string params) = 0;
		onChannelJoin(std::string channel, std::string nick) = 0;
		onChannelPart(std::string channel, std::string nick) = 0;
		onUserQuit(std::string nick) = 0;
		onChannelKick(std::string channel, std::string kicker, std::string kickee) = 0;
	private:
		std::tr1::unordered_map<std::string, Channel> *channelList;
};