CC = g++
LIBS = -ldl -lpthread
LDFLAGS=$(LIBS) -rdynamic
CXXFLAGS=-Wall -pthread $(DEBUG)
SOURCES=base.cpp config.cpp modules.cpp protocol.cpp socket.cpp

default: makedepend robobo modules

makedepend:
	for i in *.cpp; do for inc in  $(awk '/^#include "/ {print $2}' $i | sed 's/"//g'); do echo "${i%.cpp}.o: $inc"; done; done

debug:
	DEBUG="-g -O0" make -C .

robobo: $(SOURCES:.cpp=.o)

.PHONY: modules clean

modules:
	make -C modules/

clean:
	make -C modules/ clean
	rm -f $(SOURCES:.cpp=.o)