#include "socket.h"
#include "robobo.h"

class Socket {
	public:
		Socket(std::string server, unsigned short port);
		~Socket();
		void sendMsg(std::string message);
		std::string receive();
		std::vector<std::string> parseLine(std::string message);
		std::vector<std::string> parseLastParam(std::string lastParam);
		void closeConnection();
		bool isConnected();
	private:
		int socketfd, status;
		sockaddr_in socketAddr;
		char inputBuffer[2];
		std::string messageString;
		std::vector<std::string> messageParams;
		bool connected, seenCR;
};

Socket::Socket(std::string server, unsigned short port) {
	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	socketAddr.sin_family = AF_INET;
	socketAddr.sin_port = htons(port);
	inet_pton(AF_INET, server.c_str(), &socketAddr.sin_addr);
	
	fcntl(socketfd, F_SETFL, O_NONBLOCK);
	
	do
		status = connect(socketfd, (sockaddr *) &socketAddr, sizeof(socketAddr));
	while (errno == EINPROGRESS || errno == EALREADY);
	if (status != 0 && errno != EISCONN) {
		std::cout << "An error occurred connecting to the server." << std::endl;
		perror("Could not connect to server.");
		connected = false;
	} else {
		std::cout << "Successfully connected to the server." << std::endl;
		connected = true;
	}
}

Socket::~Socket() {
	closeConnection();
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

void Socket::sendMsg(std::string message) {
	message += "\r\n";
	status = send(socketfd, message.c_str(), message.size(), 0);
	if ((unsigned) status == message.size())
		std::cout << " -> " << message;
	else
		std::cout << " -!> An error occurred sending a message. " << message;
}

std::string Socket::receive() {
	messageString = "";
	seenCR = false;
	while (true) {
		status = recv(socketfd, &inputBuffer, 1, 0);
		if (status < 0 && errno != EWOULDBLOCK) {
			std::cout << status << std::endl;
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

std::vector<std::string> Socket::parseLine(std::string message) {
	messageParams.clear();
	messageString = "";
	for (unsigned int i = 0; i < message.size(); i++) {
		if (message[i] == ' ') {
			messageParams.push_back(messageString);
			messageString = "";
		} else if (message[i] == ':' && messageString == "" && i > 0) {
			for (i++; i < message.size(); i++)
				messageString += message[i];
			messageParams.push_back(messageString);
			messageString = "";
		} else
			messageString += message[i];
	}
	return messageParams;
}