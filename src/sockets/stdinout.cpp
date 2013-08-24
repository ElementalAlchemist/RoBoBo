#include "sockets.h"

class StandardInputOutput : public Socket {
	public:
		StandardInputOutput();
		unsigned int apiVersion() { return 3000; }
		std::string receive();
		void sendData(const std::string& data);
};

StandardInputOutput::StandardInputOutput() : connected(true) {}

std::string StandardInputOutput::receive() {
	std::string recvLine;
	std::getline(std::cin, recvLine);
	return recvLine;
}

void StandardInputOutput::sendData(const std::string& data) {
	std::cout << data << std::endl;
}