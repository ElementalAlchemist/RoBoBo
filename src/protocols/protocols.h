#pragma once
#include "../main.h"
#include "../servermanager.h"

#define SERVER_CLIENT_SPAWN(ServerClass) \
extern "C" bool serverType() { \
	return false; \
} \
extern "C" ClientProtocol* spawn(const std::string& name) { \
	return new ServerClass (name); \
}

#define SERVER_SERVER_SPAWN(ServerClass) \
extern "C" bool serverType() { \
	return true; \
} \
extern "C" ServerProtocol* spawn(const std::string& name) { \
	return new ServerClass (name); \
}