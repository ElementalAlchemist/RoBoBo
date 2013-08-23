$(info Checking compiler availability...)
ifeq ($(origin CC), default)
CC:=$(shell CLANGVERSION=`clang++ -dumpversion 2> /dev/null`; CLANG=$$?; GCCVERSION=`g++ -dumpversion 2> /dev/null`; GCC=$$?; if [ $$CLANG -eq 0 ]; then if [ `echo $$CLANGVERSION | cut -d '.' -f 1` -ge 4 -a `echo $$CLANGVERSION | cut -d '.' -f 2` -ge 2 ]; then echo "clang++"; exit; fi; fi; if [ $$GCC -eq 0 ]; then if [ `echo $$GCCVERSION | cut -d '.' -f 1` -ge 4 -a `echo $$GCCVERSION | cut -d '.' -f 2` -ge 9 ]; then echo "g++"; exit; fi; fi)
else # In this case, it came from the environment, so just check for sanity
CC:=$(shell ${CC} --version > /dev/null 2>&1; if [ $$? -eq 0 ]; then echo ${CC}; fi)
endif
ifdef CC
$(info Using ${CC} for building.)
$(info )
else
$(error No compiler found)
endif
CXXFLAGS?=-std=c++11 -Wall -pedantic -pthread $(DEBUG)
LDFLAGS?=-ldl -rdynamic
SOFLAGS?=-pipe -fPIC
GENDEPS?=-std=c++11 -E -MM
GENDEPSTARGET?=-MT
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
	@./module_make.sh modules > src/modules/modules.mk
	@make --no-print-directory -C src/modules

protocols:
	@test -d protocols || mkdir protocols
	@./module_make.sh protocols > src/protocols/modules.mk
	@make --no-print-directory -C src/protocols

sockets:
	@test -d sockets || mkdir sockets
	@./module_make.sh sockets > src/sockets/modules.mk
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
	@make --no-print-directory -C src/modules clean
	@make --no-print-directory -C src/protocols clean
	@make --no-print-directory -C src/sockets clean
	@rm -f src/modules/modules.mk
	@rm -f src/protocols/modules.mk
	@rm -f src/sockets/modules.mk
	@echo "Clean has been made."