#include "socket.h"

Socket::Socket() : socketfd(-1), connected(false) {}

Socket::~Socket() {}

void Socket::connectServer(std::string server, std::string port, std::string bindAddr) {}

std::string Socket::receive(bool* reset) {
	return "";
}

void Socket::sendData(std::string line) {}

void Socket::closeConnection() {}

bool Socket::isConnected() {
	return connected;
}