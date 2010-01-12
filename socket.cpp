#include "socket.h"

#ifndef SOCKET_CPP
#define SOCKET_CPP
class Socket {
	public:
		Socket();
		~Socket();
		void connectServer(std::string address, unsigned short port);
		bool sendData(std::string message);
		std::string receive();
		void closeConnection();
		bool isConnected();
	private:
		int socketfd;
		sockaddr_in socketAddr;
		bool connected;
};

Socket::Socket() {
	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	socketAddr.sin_family = AF_INET;
	connected = false;
}

Socket::~Socket() {
	closeConnection();
}

void Socket::connectServer(std::string address, unsigned short port) {
	hostent* serverHost = gethostbyname(address.c_str());
	in_addr* serverAddress = (in_addr*) serverHost->h_addr_list[0];
	socketAddr.sin_port = htons(port);
	inet_pton(AF_INET, inet_ntoa(*serverAddress), &socketAddr.sin_addr);
	int status;
	
	status = connect(socketfd, (sockaddr*) &socketAddr, sizeof(socketAddr));
	
	if (status != 0) {
		perror("Could not connect to server");
		connected = false;
	} else {
		std::cout << "Successfully connected to " << address << std::endl;
		connected = true;
	}
}

void Socket::closeConnection() {
	close(socketfd);
	connected = false;
}

bool Socket::isConnected() {
	return connected;
}

bool Socket::sendData(std::string message) {
	message += "\r\n";
	int status = send(socketfd, message.c_str(), message.size(), 0);
	if ((unsigned) status == message.size())
		return true;
	perror("An error occurred sending a message");
	return false;
}

std::string Socket::receive() {
	std::string messageString = "";
	char inputBuffer[2];
	bool seenCR = false;
	int status;
	while (true) {
		status = recv(socketfd, &inputBuffer, 1, 0);
		if (status < 0) {
			perror("An error occurred receiving a message");
			closeConnection();
			break;
		}
		if (inputBuffer[0] == '\0')
			return messageString;
		if (inputBuffer[0] == '\n' && seenCR)
			return messageString;
		if (inputBuffer[0] != '\n' && inputBuffer[0] != '\r')
			messageString += inputBuffer[0];
		seenCR = false;
		if (inputBuffer[0] == '\r')
			seenCR = true;
	}
	return "";
}
#endif