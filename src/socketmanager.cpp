#include "socketmanager.h"

std::shared_ptr<Socket> SocketManager::getSocket(const std::string& socketType) {
	void* sockFile = dlopen(("sockets/" + socketType + ".so").c_str(), RTLD_NOW);
	if (sockFile == nullptr)
		throw SocketLoadFailed (dlerror());
	void* spawnFunc = dlsym(sockFile, "spawn");
	if (spawnFunc == nullptr) {
		const char* loadError = dlerror();
		dlclose(sockFile);
		if (loadError)
			throw SocketLoadFailed (loadError);
		throw SocketLoadFailed ("The spawn symbol has been set to null, but it must be a valid function.");
	}
	Socket*(*spawnCallFunc)() = reinterpret_cast<Socket*(*)()>(spawnFunc);
	std::shared_ptr<Socket> newSocket (spawnCallFunc(), std::bind(&SocketManager::removeSocket, this, socketType, sockFile, std::placeholders::_1));
	if (sockAPIVersions.find(newSocket->apiVersion()) == sockAPIVersions.end())
		throw SocketAPIMismatch ();
	return newSocket;
}

void SocketManager::removeSocket(const std::string& socketType, void* sockFile, Socket* deletingSocket) {
	delete deletingSocket;
	dlclose(sockFile);
	LogManager* logger = LogManager::getHandle();
	logger->log(LOG_DEBUG, "socket-request", "A " + socketType + " socket has been unloaded.");
}