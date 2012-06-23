#include "socket.h"

class Plaintext : public Socket {
	public:
		Plaintext();
		~Plaintext();
		unsigned int apiVersion();
		void connectServer(std::string server, std::string port, std::string bindAddr = "");
		std::string receive(bool* reset);
		void sendData(std::string line);
		void closeConnection();
};

Plaintext::Plaintext() : Socket() {}

Plaintext::~Plaintext() {
	closeConnection();
}

unsigned int Plaintext::apiVersion() {
	return 3000;
}

void Plaintext::connectServer(std::string server, std::string port, std::string bindAddr) {
	addrinfo* addrInfoList;
	addrinfo hints;
	hints.ai_family = PF_UNSPEC; // Don't specify either IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // IRC uses TCP, which is streaming
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_ADDRCONFIG | AI_NUMERICHOST; // AI_ADDRCONFIG doesn't allow IPv4 when the machine doesn't have IPv4, same with IPv6; AI_NUMERICHOST signals a numeric port
	int status = getaddrinfo(server.c_str(), port.c_str(), &hints, &addrInfoList);
	if (status != 0) {
		std::cerr << "An error occurred getting hosts for the server " << server << " on port " << port << "." << std::endl;
		return;
	}
	for (addrinfo* thisAddr = addrInfoList; thisAddr != NULL; thisAddr = thisAddr->ai_next) {
		socketfd = socket(thisAddr->ai_family, thisAddr->ai_socktype | SOCK_NONBLOCK, thisAddr->ai_protocol);
		if (socketfd == -1)
			continue;
		if (bindAddr != "") {
			status = bind(socketfd, bindAddr.c_str(), bindAddr.size());
			if (status == -1) {
				close(socketfd);
				socketfd = -1;
				continue;
			}
		}
		status = connect(socketfd, thisAddr->ai_addr, thisAddr->ai_addrlen);
		if (status != 0 && errno != EINPROGRESS) {
			close(socketfd);
			socketfd = -1;
			continue;
		}
		break; // If we haven't continued out yet, we're connected and the socket is suitable.
	}
	freeaddrinfo(addrInfoList);
	if (socketfd == -1) {
		std::cerr << "No suitable host was found for the server " << server << " on port " << port << "." << std::endl;
		return;
	}
	connected = true;
}

std::string Plaintext::receive(bool* reset) {
	char inputBuffer[2];
	std::string incomingMsg = "";
	bool shouldSleep;
	int status;
	while (true) {
		shouldSleep = false;
		do {
			if (*reset)
				return "";
			if (shouldSleep)
				usleep(50000);
			status = recv(socketfd, &inputBuffer, 1, 0);
			shouldSleep = true;
		} while (status < 0 && (errno == EAGAIN || errno == EWOULDBLOCK));
		if (status <= 0) {
			close(socketfd);
			connected = false;
			return "";
		}
		if (inputBuffer[0] == '\r')
			continue;
		if (inputBuffer[0] == '\n')
			break;
		incomingMsg += inputBuffer[0];
	}
	return incomingMsg;
}

void Plaintext::sendData(std::string line) {
	line += "\r\n";
	int status;
	do
		status = send(socketfd, line.c_str(), line.size(), 0);
	while (status != line.size() && (errno == EAGAIN || errno == EWOULDBLOCK));
}

void Plaintext::closeConnection() {
	close(socketfd);
	connected = false;
}

SOCKET_SPAWN(Plaintext)