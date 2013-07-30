.DEFAULT: default

.PHONY: default clean

default: ../robobo

../robobo: config.o globpattern.o ircmessage.o logmanager.o modulemanager.o mutexlocker.o servermanager.o socketmanager.o robobo.cpp
	@echo "\033[1;32mLink:\033[0m\trobobo"
	@${CC} ${CXXFLAGS} ${DEBUG} config.o globpattern.o ircmessage.o logmanager.o modulemanager.o mutexlocker.o servermanager.o socketmanager.o robobo.cpp -o ../robobo ${LDFLAGS}

clean:
	@rm -f *.o

# include compiler-generated dependencies
$(shell ${CC} ${GENDEPS} config.cpp)
	@echo "\033[1;32mBuild:\033[0m\tconfig.cpp"
	@${CC} ${CXXFLAGS} ${DEBUG} config.cpp -c -o config.o

$(shell ${CC} ${GENDEPS} globpattern.cpp)
	@echo "\033[1;32mBuild:\033[0m\tglobpattern.cpp"
	@${CC} ${CXXFLAGS} ${DEBUG} globpattern.cpp -c -o globpattern.o

$(shell ${CC} ${GENDEPS} ircmessage.cpp)
	@echo "\033[1;32mBuild:\033[0m\tircmessage.cpp"
	@${CC} ${CXXFLAGS} ${DEBUG} ircmessage.cpp -c -o ircmessage.o

$(shell ${CC} ${GENDEPS} logmanager.cpp)
	@echo "\033[1;32mBuild:\033[0m\tlogmanager.cpp"
	@${CC} ${CXXFLAGS} ${DEBUG} logmanager.cpp -c -o logmanager.cpp

$(shell ${CC} ${GENDEPS} modulemanager.cpp)
	@echo "\033[1;32mBuild:\033[0m\tmodulemanager.cpp"
	@${CC} ${CXXFLAGS} ${DEBUG} modulemanager.cpp -c -o modulemanager.cpp

$(shell ${CC} ${GENDEPS} mutexlocker.cpp)
	@echo "\033[1;32mBuild:\033[0m\tmutexlocker.cpp"
	@${CC} ${CXXFLAGS} ${DEBUG} mutexlocker.cpp -c -o mutexlocker.cpp

$(shell ${CC} ${GENDEPS} servermanager.cpp)
	@echo "\033[1;32mBuild:\033[0m\tservermanager.cpp"
	@${CC} ${CXXFLAGS} ${DEBUG} servermanager.cpp -c -o servermanager.cpp

$(shell ${CC} ${GENDEPS} socketmanager.cpp)
	@echo "\033[1;32mBuild:\033[0m\tsocketmanager.cpp"
	@${CC} ${CXXFLAGS} ${DEBUG} socketmanager.cpp -c -o socketmanager.cpp