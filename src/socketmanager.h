#pragma once
#include "main.h"
#include "logmanager.h"
#include "modtypes/socket.h"

const std::set<unsigned int> sockAPIVersions { 3000 };

class SocketManager {
	public:
		std::shared_ptr<Socket> getSocket(const std::string& socketType);
	private:
		void removeSocket(const std::string& socketType, void* sockFile, Socket* deletingSocket);
};

class SocketLoadFailed : public std::exception {
	public:
		SocketLoadFailed(std::string&& desc) : description(std::forward<std::string>(desc)) {}
		const char* what() const noexcept { return description.c_str(); }
	private:
		std::string description;
};

class SocketAPIMismatch : public std::exception {
	public:
		const char* what() const noexcept { return "The socket module is not compatible with the current module API."; }
};

class SocketOperationFail : public std::exception {
	public:
		SocketConnectFail(std::string&& desc) : description(std::forward<std::string>(desc)) {}
		const char* what() const noexcept { return description.c_str(); }
	private:
		std::string description;
};