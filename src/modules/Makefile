CC=g++
CXXFLAGS=-std=c++0x -Wall -pedantic -pthread -rdynamic $(DEBUG)
# CXXFLAGS includes -pthread because g++ still requires it for std::thread
LDFLAGS=-ldl
SOFLAGS=-pipe -fPIC -shared
OBJECTS=$(shell for source in $(basename $(wildcard *.cpp)); do printf " ../../modules/$$source.so"; done)

.DEFAULT: default

.PHONY: default

default: ${OBJECTS}

include modules.mk