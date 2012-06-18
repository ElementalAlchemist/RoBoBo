CC=g++
CXXFLAGS=-std=c++0x -Wall -pedantic -pthread -rdynamic $(DEBUG)
# CXXFLAGS includes -pthread because g++ still requires it for std::thread
LDFLAGS=-ldl
SOFLAGS=-pipe -fPIC -shared
SOURCES=$(wildcard *.cpp)

.DEFAULT: default

.PHONY: default

default: $(SOURCES:.cpp=.so)
	@mv *.so ../../modules/

$(SOURCES:.cpp=.so):
	@${CC} ${CXXFLAGS} ${SOFLAGS} $(@:.so=.cpp) ${LDFLAGS} $(shell grep '^\s*// COMPILE_FLAGS ' $(@:.so=.cpp) | awk -F'[ ]' '{$$1=$$2=""; print $0}') -c -o $@