CC = g++
LIBS = -ldl -lpthread
LDFLAGS=$(LIBS) -rdynamic
CXXFLAGS=-Wall -W -pthread $(DEBUG)

default: modules robobo

debug:
	DEBUG="-g -O0" make -C .

#robobo: modules/m_ctcppingreply.so
#robobo: modules/m_ctcptimereply.so
#robobo: modules/m_admin.so
#robobo: modules/m_admin_loadmod.so
robobo: robobo.cpp

.PHONY: modules clean

modules:
	make -C modules/

clean:
	make -C modules/ clean
	rm robobo
