CC = g++
LIBS = -ldl -lpthread
LDFLAGS=$(LIBS) -rdynamic
CXXFLAGS=-Wall -pthread $(DEBUG)
SOURCES=modinterface.cpp server.cpp channel.cpp config.cpp modules.cpp server.cpp socket.cpp user.cpp
 
default: makedepend robobo

makedepend:
	for i in *.cpp; do for inc in  $(awk '/^#include "/ {print $2}' $i | sed 's/"//g'); do echo "${i%.cpp}.o: $inc"; done; done > include.mk
 
debug:
	DEBUG="-g -O0" make -C .

robobo: $(SOURCES:.cpp=.o)
 
.PHONY: modules clean
 
modules:
	make -C modules/
 
clean:
	make -C modules/ clean
	rm robobo
include include.mk
