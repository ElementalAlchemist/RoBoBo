.DEFAULT: default

.PHONY: default clean

default: ../../../protocols/client.so

../../../protocols/client.so: user.o client.o channel.o protocol.o
	@echo "\033[1;32mLink:\033[0m\tclient.so"
	@${CC} ${CXXFLAGS} ${DEBUG} -shared user.o client.o channel.o protocol.o -o ../../../protocols/client.so ${LDFLAGS}

$(shell ${CC} ${GENDEPS} user.cpp | tr "\\\\\\n" " " | sed -e 's/\s\s*/ /g')
	@echo "\033[1;32mBuild:\033[0m\tuser.cpp"
	@${CC} ${CXXFLAGS} ${DEBUG} user.cpp -c -o user.o

$(shell ${CC} ${GENDEPS} client.cpp | tr "\\\\\\n" " " | sed -e 's/\s\s*/ /g')
	@echo "\033[1;32mBuild:\033[0m\tclient.cpp"
	@${CC} ${CXXFLAGS} ${DEBUG} client.cpp -c -o client.o

$(shell ${CC} ${GENDEPS} channel.cpp | tr "\\\\\\n" " " | sed -e 's/\s\s*/ /g')
	@echo "\033[1;32mBuild:\033[0m\tchannel.cpp"
	@${CC} ${CXXFLAGS} ${DEBUG} channel.cpp -c -o channel.o

$(shell ${CC} ${GENDEPS} protocol.cpp | tr "\\\\\\n" " " | sed -e 's/\s\s*/ /g')
	@echo "\033[1;32mBuild:\033[0m\tprotocol.cpp"
	@${CC} ${CXXFLAGS} ${DEBUG} protocol.cpp -c -o protocol.o