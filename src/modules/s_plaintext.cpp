#include "sockinclude.h"

class PlainText : public Socket {
	public:
		unsigned int apiVersion();
		bool bindSocket(std::string address);
		void connectServer(std::string address, unsigned short port);
		void closeConnection();
		bool sendData(std::string message);
		std::string receive();
};

unsigned int PlainText::apiVersion() {
	return 2000;
}

bool PlainText::bindSocket(std::string address) {
	int trueint = 1;
	if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &trueint, sizeof(trueint)) < 0) {
		perror("Could not apply socket settings");
		return false;
	}
	sockaddr_in bindAddr;
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_port = htons(0);
	inet_pton(AF_INET, address.c_str(), &bindAddr.sin_addr);
	int status = bind(socketfd, (sockaddr*) &bindAddr, sizeof(bindAddr));
	if (status < 0) {
		perror("Could not bind to IP address");
		return false;
	}
	return true;
}

void PlainText::connectServer(std::string address, unsigned short port) {
	hostent* serverHost = gethostbyname(address.c_str());
	in_addr* serverAddress = (in_addr*) serverHost->h_addr_list[0];
	socketAddr.sin_port = htons(port);
	inet_pton(AF_INET, inet_ntoa(*serverAddress), &socketAddr.sin_addr);
	int status;
	
	status = connect(socketfd, (sockaddr*) &socketAddr, sizeof(socketAddr));
	
	if (status != 0 && errno != EINPROGRESS) {
		perror("Could not connect to server");
		connected = false;
	} else {
		std::cout << "Successfully connected to " << address << std::endl;
		connected = true;
	}
}

void PlainText::closeConnection() {
	close(socketfd);
	connected = false;
}

bool PlainText::sendData(std::string message) {
	message += "\r\n";
	int status;
	do
		status = send(socketfd, message.c_str(), message.size(), 0);
	while (status < 0 && errno == EAGAIN);
	if ((unsigned) status == message.size())
		return true;
	perror("An error occurred sending a message");
	return false;
}

std::string PlainText::receive() {
	std::string messageString = "";
	char inputBuffer[2];
	int status;
	bool addPause = false;
	while (true) {
		do {
			if (addPause)
				usleep(100000);
			status = recv(socketfd, &inputBuffer, 1, 0);
			addPause = true;
		} while ((status < 0 && errno == EWOULDBLOCK) && status != 0);
		addPause = false;
		if (status <= 0) {
			perror("An error occurred receiving a message");
			closeConnection();
			break;
		}
		if (inputBuffer[0] == '\n')
			return messageString;
		if (inputBuffer[0] != '\r')
			messageString += inputBuffer[0];
	}
	return "";
}