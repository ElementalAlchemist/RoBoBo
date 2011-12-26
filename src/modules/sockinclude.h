#include "../main.h"
#include "../socket.h"

#define SOCKET_SPAWN(name) extern "C" Socket* spawn() {\
	return new name;\
}