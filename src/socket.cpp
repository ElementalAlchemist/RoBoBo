#include "socket.h"

Socket::Socket() : socketfd(socket(AF_INET, SOCK_STREAM, 0)), connected(false) {
	socketAddr.sin_family = AF_INET;
	fcntl(socketfd, F_SETFL, O_NONBLOCK);
}

Socket::~Socket() {
	closeConnection();
}

bool Socket::bindSocket(std::string address) { return false; }

void Socket::closeConnection() {}

bool Socket::isConnected() { return connected; }

bool Socket::sendData(std::string message) { return false; }

std::string Socket::receive() { return ""; }

void Socket::connectServer(std::string address, unsigned short port) {}