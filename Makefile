CC = g++
LIBS = -ldl

default:
	make all
debug:
	g++ -ansi -pedantic-errors -Wall -ldl -pthread -g -O0 -fno-inline robobo.cpp -o robobo
	g++ -ansi -pedantic-errors -Wall -c -g -export-dynamic -pipe -shared -fPIC modules/m_helloworld.cpp -o modules/m_helloworld.so
all:
	g++ -ansi -pedantic-errors -Wall -ldl -pthread robobo.cpp -o robobo
	g++ -ansi -pedantic-errors -Wall -c -export-dynamic -pipe -shared -fPIC modules/m_helloworld.cpp -o modules/m_helloworld.so
clean:
	rm -f *.so *.o
	rm robobo