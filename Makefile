CC=g++
CXXFLAGS=-std=c++0x -Wall -pedantic -pthread -rdynamic $(DEBUG)
# CXXFLAGS includes -pthread because g++ still requires it for std::thread
LDFLAGS=-ldl

.PHONY: default debug core modules clean

.DEFAULT: default

default: core modules

debug: debugcore debugmodules

core:
	@make -C src

modules:
	@test -d modules || mkdir modules
	@make -C src/modules

debugcore:
	@DEBUG="-g -O0" make -C src

debugmodules:
	@test -d modules || mkdir modules
	@DEBUG="-g -O0" make -C src/modules

clean:
	@rm -f robobo
	@rm -f modules/*
	@make -C src clean
	@echo "Clean has been made."