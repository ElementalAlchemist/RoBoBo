$(info Checking compiler availability...)
CC:=$(shell clang++ --version > /dev/null 2>&1; CLANG=$$?; g++ --version > /dev/null 2>&1; GCC=$$?; if [ $$CLANG -eq 0 ]; then echo "clang++"; elif [ $$GCC -eq 0 ]; then echo "g++"; fi)
ifdef CC
$(info Using ${CC})
else
$(error No compiler found)
endif
CXXFLAGS=-std=c++11 -Wall -pedantic -pthread $(DEBUG)
LDFLAGS=-ldl -rdynamic
SOFLAGS=-pipe -fPIC -shared
GENDEPS=-std=c++11 -E -MM
GENDEPSTARGET=-MT
SHELL:=/bin/bash
export

.PHONY: default debug core modules protocols sockets debugcore debugmodules debugprotocols debugsockets clean

.DEFAULT: default

default: core modules protocols sockets

debug: debugcore debugmodules debugprotocols debugsockets

core:
	@make --no-print-directory -C src
	@test -d logs || mkdir logs

modules:
	@test -d modules || mkdir modules
	@src/modules/module_make.sh > src/modules/modules.mk
	@make --no-print-directory -C src/modules

protocols:
	@test -d protocols || mkdir protocols
	@src/protocols/module_make.sh > src/protocols/modules.mk
	@make --no-print-directory -C src/protocols

sockets:
	@test -d sockets || mkdir sockets
	@src/sockets/module_make.sh > src/sockets/modules.mk
	@make --no-print-directory -C src/sockets

debugcore:
	@DEBUG="-g -O0" make --no-print-directory -C src
	@test -d logs || mkdir logs

debugmodules:
	@test -d modules || mkdir modules
	@DEBUG="-g -O0" src/modules/module_make.sh > src/modules/modules.mk
	@DEBUG="-g -O0" make --no-print-directory -C src/modules

debugprotocols:
	@test -d protocols || mkdir protocols
	@DEBUG="-g -O0" src/protocols/module_make.sh > src/protocols/modules.mk
	@DEBUG="-g -O0" make --no-print-directory -C src/protocols

debugsockets:
	@test -d sockets || mkdir sockets
	@DEBUG="-g -O0" src/sockets/module_make.sh > src/sockets/modules.mk
	@DEBUG="-g -O0" make --no-print-directory -C src/sockets

clean:
	@rm -f robobo
	@rm -f modules/*
	@rm -f protocols/*
	@rm -f sockets/*
	@make --no-print-directory -C src clean
	@rm -f src/modules/modules.mk
	@rm -f src/protocols/modules.mk
	@rm -f src/sockets/modules.mk
	@echo "Clean has been made."