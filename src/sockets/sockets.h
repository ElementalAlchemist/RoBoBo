#pragma once
#include "../main.h"
#include "../modtypes/socket.h"
#include "../socketmanager.h"

#define SOCKET_SPAWN(SocketClass) \
extern "C" Socket* spawn() { \
	return new SocketClass; \
}