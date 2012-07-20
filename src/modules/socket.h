#include "../main.h"
#include "../socket.h"

#define SOCKET_SPAWN(modName) extern "C" Socket* spawn() {\
	return new modName;\
}
