#include "sockets.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

class Plaintext : public Socket {
	public:
		unsigned int apiVersion() { return 3000; }
		void connectServer(const std::string& server, const std::string& port, const std::string& bindAddr = "");
		std::string receive();
		void sendData(const std::string& data);
		void closeConnection();
	private:
		int socketfd;
		std::ostringstream recvBuffer;
};

void Plaintext::connectServer(const std::string& server, const std::string& port, const std::string& bindAddr) {
	addrinfo* addrInfoList;
	addrinfo hints;
	hints.ai_family = PF_UNSPEC; // Don't specify whether IPv6 or IPv4
	hints.ai_socktype = SOCK_STREAM; // IRC uses TCP, so make a streaming socket
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_ADDRCONFIG | AI_NUMERICHOST;
	// AI_ADDRCONFIG doesn't allow IPv4 when the machine doesn't have IPv4 or IPv6 if the machine doesn't have IPv6
	// AI_NUMERICHOST signals that we're passing a numeric port
	int status = getaddrinfo(server.c_str(), port.c_str(), &hints, &addrInfoList);
	if (status != 0)
		throw SocketOperationFailed ("An error occurred getting the hosts.");
	for (addrinfo* thisAddr = addrInfoList; thisAddr != nullptr; thisAddr = thisAddr->ai_next) {
		socketfd = socket(thisAddr->ai_family, thisAddr->ai_socktype | SOCK_NONBLOCK, thisAddr->ai_protocol);
		if (socketfd == -1)
			continue;
		if (!bindAddr.empty()) {
			addrinfo bindHints;
			bindHints.ai_family = thisAddr->ai_family;
			bindHints.ai_socktype = SOCK_STREAM;
			bindHints.ai_protocol = IPPROTO_TCP;
			bindHints.ai_flags = AI_ADDRCONFIG | AI_NUMERICHOST;
			addrinfo* bindLoc;
			status = getaddrinfo(bindAddr.c_str(), "0", &bindHints, &bindLoc);
			if (status != 0) {
				close(socketfd);
				socketfd = -1;
				continue;
			}
			for (addrinfo* currBind = bindLoc; currBind != nullptr; currBind = currBind->ai_next) {
				status = bind(socketfd, currBind->ai_addr, currBind->ai_addrlen);
				if (status == -1)
					continue;
			}
			freeaddrinfo(bindLoc);
			if (status != 0) { // If the status still isn't 0, then the binding never worked
				close(socketfd);
				socketfd = -1;
				freeaddrinfo(addrInfoList);
				throw SocketOperationFailed ("The socket could not be bound.");
			}
		}
		status = connect(socketfd, thisAddr->ai_addr, thisAddr->ai_addrlen);
		if (status != 0 && errno != EINPROGRESS) {
			close(socketfd);
			socketfd = -1;
			continue;
		}
		break; // If we haven't continued yet, we're connected and the socket is suitable.
	}
	freeaddrinfo(addrInfoList);
	if (socketfd == -1)
		throw SocketOperationFailed ("No suitable host was found.");
	connected = true;
}

std::string Plaintext::receive() {
	std::string bufferStr (recvBuffer.str());
	size_t delimPos = bufferStr.find(delimiter);
	if (delimPos != std::string::npos) {
		std::string nextLine (bufferStr.substr(0, delimPos));
		if (delimPos + delimiter.size() < bufferStr.size())
			recvBuffer.str(bufferStr.substr(delimPos + delimiter.size()));
		else
			recvBuffer.str("");
		return nextLine;
	}
	char inputBuffer[1025];
	int status;
	while (true) {
		status = recv(socketfd, &inputBuffer, 1024, 0);
		while (status < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
			std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Give some (although brief) wait before checking again
			status = recv(socketfd, &inputBuffer, 1024, 0);
		}
		if (status <= 0) {
			close(socketfd);
			connected = false;
			throw SocketOperationFailed ("The connection has broken.");
		}
		recvBuffer << inputBuffer;
		bufferStr = recvBuffer.str();
		delimPos = bufferStr.find(delimiter);
		if (delimPos != std::string::npos) {
			std::string nextLine (bufferStr.substr(0, delimPos));
			if (delimPos + delimiter.size() < bufferStr.size())
				recvBuffer.str(bufferStr.substr(delimPos + delimiter.size()));
			else
				recvBuffer.str("");
			return nextLine;
		}
	}
}

void Plaintext::sendData(const std::string& data) {
	std::string line (data + delimiter);
	ssize_t status;
	do
		status = send(socketfd, line.c_str(), line.size(), 0);
	while (status <= 0 && (errno == EAGAIN || errno == EWOULDBLOCK));
}

void Plaintext::closeConnection() {
	close(socketfd);
	connected = false;
}

SOCKET_SPAWN(Plaintext)