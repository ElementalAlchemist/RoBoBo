#pragma once
#include "../main.h"
#include "../modulemanager.h"

#define MODULE_SPAWN(ModuleClass) \
extern "C" Module* spawn(const std::string& name) { \
	return new ModuleClass (name); \
}