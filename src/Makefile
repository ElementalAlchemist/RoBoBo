CC=g++
CXXFLAGS=-std=c++0x -Wall -pedantic -pthread -rdynamic $(DEBUG)
# CXXFLAGS includes -pthread because g++ still requires it for std::thread
LDFLAGS=-ldl

.DEFAULT: default

.PHONY: default clean

default: base.o module.o protocol.o socket.o
	@$(CC) $(CXXFLAGS) $(LDFLAGS) $(DEBUG) $(SOURCES:.cpp=.o) robobo.cpp -o ../robobo

clean:
	rm *.o

# include g++-generated dependencies
$(shell ${CC} -std=c++0x -E -MM base.cpp)
	@${CC} ${CXXFLAGS} ${DEBUG} base.cpp ${LDFLAGS} -o base.o 

$(shell ${CC} -std=c++0x -E -MM module.cpp)
	@${CC} ${CXXFLAGS} ${DEBUG} module.cpp ${LDFLAGS} -o module.o

$(shell ${CC} -std=c++0x -E -MM protocol.cpp)
	@${CC} ${CXXFLAGS} ${DEBUG} protocol.cpp ${LDFLAGS} -o protocol.o

$(shell ${CC} -std=c++0x -E -MM socket.cpp)
	@${CC} ${CXXFLAGS} ${DEBUG} socket.cpp ${LDFLAGS} -o socket.o