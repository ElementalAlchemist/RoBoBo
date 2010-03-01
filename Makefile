CC = g++
LIBS = -ldl

default:
	make robobo
debug:
	g++ -ansi -pedantic-errors -Wall -ldl -pthread -rdynamic -g -O0 -fno-inline robobo.cpp -o robobo
	g++ -ansi -pedantic-errors -Wall -pipe -shared -fPIC -g modules/m_ctcpversionreply.cpp -o modules/m_ctcpversionreply.so
	g++ -ansi -pedantic-errors -Wall -pipe -shared -fPIC -g modules/m_ctcppingreply.cpp -o modules/m_ctcppingreply.so
	g++ -ansi -pedantic-errors -Wall -pipe -shared -fPIC -g modules/m_ctcptimereply.cpp -o modules/m_ctcptimereply.so
	g++ -ansi -pedantic-errors -Wall -pipe -shared -fPIC -g modules/m_dccchat.cpp -o modules/m_dccchat.so
	g++ -ansi -pedantic-errors -Wall -pipe -shared -fPIC -g modules/m_admin.cpp -o modules/m_admin.so
	g++ -ansi -pedantic-errors -Wall -pipe -shared -fPIC -g modules/m_admin_loadmod.cpp -o modules/m_admin_loadmod.so
robobo:
	g++ -ansi -pedantic-errors -Wall -ldl -pthread -rdynamic robobo.cpp -o robobo
	g++ -ansi -pedantic-errors -Wall -pipe -shared -fPIC modules/m_ctcpversionreply.cpp -o modules/m_ctcpversionreply.so
	g++ -ansi -pedantic-errors -Wall -pipe -shared -fPIC modules/m_ctcppingreply.cpp -o modules/m_ctcppingreply.so
	g++ -ansi -pedantic-errors -Wall -pipe -shared -fPIC modules/m_ctcptimereply.cpp -o modules/m_ctcptimereply.so
	g++ -ansi -pedantic-errors -Wall -pipe -shared -fPIC modules/m_admin.cpp -o modules/m_admin.so
	g++ -ansi -pedantic-errors -Wall -pipe -shared -fPIC modules/m_admin_loadmod.cpp -o modules/m_admin_loadmod.so
clean:
	rm modules/*.so
	rm robobo