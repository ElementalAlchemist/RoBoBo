CC = g++
LIBS = -ldl -lpthread
LDFLAGS=$(LIBS) -rdynamic
CXXFLAGS=-Wall -pthread $(DEBUG)
 
default: modules robobo
 
debug:
	DEBUG="-g -O0" make -C .

robobo: robobo.cpp
 
.PHONY: modules clean
 
modules:
	make -C modules/
 
clean:
	make -C modules/ clean
	rm robobo