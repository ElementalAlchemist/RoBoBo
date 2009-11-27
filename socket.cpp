#include "socket.h"

#ifndef SOCKET_ROBOBO
#define SOCKET_ROBOBO
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
	fcntl(socketfd, F_SETFL, O_NONBLOCK);
	connected = false;
}

Socket::~Socket() {
	closeConnection();
}

void Socket::connectServer(std::string address, unsigned short port) {
	socketAddr.sin_port = htons(port);
	inet_pton(AF_INET, address.c_str(), &socketAddr.sin_addr);
	int status;
	
	// TODO: resolve DNS
	
	do
		status = connect(socketfd, (sockaddr*) &socketAddr, sizeof(socketAddr));
	while (errno == EINPROGRESS || errno == EALREADY);
	if (status != 0 && errno != EISCONN) {
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
	if (connected)
		return true;
	return false;
}

bool Socket::sendData(std::string message) {
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
		if (status < 0 && errno != EWOULDBLOCK) {
			perror("An error occurred receiving a message");
			closeConnection();
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
}
#endif