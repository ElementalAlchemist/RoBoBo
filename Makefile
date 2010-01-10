CC = g++
LIBS = -ldl

default:
	make robobo
debug:
	g++ -ansi -pedantic-errors -Wall -ldl -pthread -rdynamic -g -O0 -fno-inline robobo.cpp -o robobo
	g++ -ansi -pedantic-errors -Wall -pipe -shared -fPIC -g modules/m_helloworld.cpp -o modules/m_helloworld.so
	g++ -ansi -pedantic-errors -Wall -pipe -shared -fPIC -g modules/m_modetest.cpp -o modules/m_modetest.so
robobo:
	g++ -ansi -pedantic-errors -Wall -ldl -pthread -rdynamic robobo.cpp -o robobo
	g++ -ansi -pedantic-errors -Wall -pipe -shared -fPIC modules/m_helloworld.cpp -o modules/m_helloworld.so
	g++ -ansi -pedantic-errors -Wall -pipe -shared -fPIC modules/m_modetest.cpp -o modules/m_modetest.so
clean:
	rm modules/*.so
	rm robobo