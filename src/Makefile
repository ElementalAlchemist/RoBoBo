CC=g++
CXXFLAGS=-std=c++0x -Wall -pedantic -pthread -rdynamic $(DEBUG)
# CXXFLAGS includes -pthread because g++ still requires it for std::thread
LDFLAGS=-ldl

.DEFAULT: default

.PHONY: default clean

default: base.o module.o protocol.o socket.o
	@echo "\033[1;32mLink:\033[0m\trobobo\033[0;33m"
	@$(CC) $(CXXFLAGS) $(DEBUG) base.cpp module.cpp protocol.cpp socket.cpp robobo.cpp -o ../robobo $(LDFLAGS)

clean:
	@rm -f *.o

# include g++-generated dependencies
$(shell ${CC} -std=c++0x -E -MM base.cpp)
	@echo "\033[1;32mBuild:\033[0m\tbase.cpp\033[0;33m"
	@${CC} ${CXXFLAGS} ${DEBUG} base.cpp ${LDFLAGS} -c -o base.o 

$(shell ${CC} -std=c++0x -E -MM module.cpp)
	@echo "\033[1;32mBuild:\033[0m\tmodule.cpp\033[0;33m"
	@${CC} ${CXXFLAGS} ${DEBUG} module.cpp ${LDFLAGS} -c -o module.o

$(shell ${CC} -std=c++0x -E -MM protocol.cpp)
	@echo "\033[1;32mBuild:\033[0m\tprotocol.cpp\033[0;33m"
	@${CC} ${CXXFLAGS} ${DEBUG} protocol.cpp ${LDFLAGS} -c -o protocol.o

$(shell ${CC} -std=c++0x -E -MM socket.cpp)
	@echo "\033[1;32mBuild:\033[0m\tsocket.cpp\033[0;33m"
	@${CC} ${CXXFLAGS} ${DEBUG} socket.cpp ${LDFLAGS} -c -o socket.o