#include "socket.h"


Socket::Socket() {
	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	socketAddr.sin_family = AF_INET;
	connected = false;
	sockTimeout.tv_sec = 150;
	sockTimeout.tv_usec = 0;
	FD_ZERO(&fdlist);
	FD_SET(socketfd, &fdlist);
	fcntl(socketfd, F_SETFL, O_NONBLOCK);
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
	
	if (status != 0 && errno != EINPROGRESS) {
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
	int status = select(socketfd + 1, NULL, &fdlist, NULL, &sockTimeout);
	if (status < 0) {
		perror("An error occurred sending a message");
		return false;
	}
	status = send(socketfd, message.c_str(), message.size(), 0);
	if ((unsigned) status == message.size())
		return true;
	perror("An error occurred sending a message");
	return false;
}

std::string Socket::receive() {
	std::string messageString = "";
	char inputBuffer[2];
	int status;
	while (true) {
		status = select(socketfd + 1, &fdlist, NULL, NULL, &sockTimeout);
		if (status < 0) {
			perror("An error occurred receiving a message");
			closeConnection();
			break;
		}
		if (status == 0)
			continue;
		status = recv(socketfd, &inputBuffer, 1, 0);
		if (status < 0) {
			perror("An error occurred receiving a message");
			closeConnection();
			break;
		}
		if (inputBuffer[0] == '\0')
			return messageString;
		if (inputBuffer[0] == '\n')
			return messageString;
		if (inputBuffer[0] != '\n' && inputBuffer[0] != '\r')
			messageString += inputBuffer[0];
	}
	return "";
}