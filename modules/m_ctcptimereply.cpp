#include "modinclude.h"
#include <ctime>

class m_ctcptimereply : public Module {
	public:
		void onChannelCTCP(std::string server, std::string channel, char target, std::string nick, std::string message);
		void onUserCTCP(std::string server, std::string nick, std::string message);
	private:
		void sendTime(std::string server, std::string target);
};

void m_ctcptimereply::onChannelCTCP(std::string server, std::string channel, char target, std::string nick, std::string message) {
	if (splitBySpace(message)[0] == "TIME")
		sendTime(server, nick);
}

void m_ctcptimereply::onUserCTCP(std::string server, std::string nick, std::string message) {
	if (splitBySpace(message)[0] == "TIME")
		sendTime(server, nick);
}

void m_ctcptimereply::sendTime(std::string server, std::string target) {
	time_t unixTimestamp = time(NULL);
	tm* currTime = localtime(&unixTimestamp);
	std::string month;
	switch (currTime->tm_mon) {
		case 0:
			month = "January";
			break;
		case 1:
			month = "February";
			break;
		case 2:
			month = "March";
			break;
		case 3:
			month = "April";
			break;
		case 4:
			month = "May";
			break;
		case 5:
			month = "June";
			break;
		case 6:
			month = "July";
			break;
		case 7:
			month = "August";
			break;
		case 8:
			month = "September";
			break;
		case 9:
			month = "October";
			break;
		case 10:
			month = "November";
			break;
		case 11:
			month = "December";
			break;
	}
	int year = currTime->tm_year + 1900;
	std::ostringstream timeOutput;
	timeOutput << month << " " << currTime->tm_mday << " " << year << " " << currTime->tm_hour << ":";
	if (currTime->tm_min < 10)
		timeOutput << "0";
	timeOutput << currTime->tm_min << ":";
	if (currTime->tm_sec < 10)
		timeOutput << "0";
	timeOutput << currTime->tm_sec << " ";
	timeOutput << "(" << unixTimestamp << ")";
	sendCTCPReply(server, target, "TIME", timeOutput.str());
}

extern "C" Module* spawn() {
	return new m_ctcptimereply;
}