CXX = g++
CXXFLAGS=-fPIC -Wall -pipe -ansi -pedantic-errors $(DEBUG)
LDFLAGS=-shared
DEPS=modinclude.h ../main.h ../connection.h ../modules.h ../config.cpp ../modules.cpp ../modinterface.cpp
 
all: modules
 
modules: m_ctcppingreply.so m_ctcpversionreply.so m_ctcptimereply.so m_dccchat.so m_admin.so m_admin_loadmod.so m_admin_unloadmod.so m_admin_rehash.so
 
m_ctcppingreply.so: m_ctcppingreply.o
	$(CXX) $(LDFLAGS) $^ -o $@
m_ctcpversionreply.so: m_ctcpversionreply.o
	$(CXX) $(LDFLAGS) $^ -o $@
m_ctcptimereply.so: m_ctcptimereply.o
	$(CXX) $(LDFLAGS) $^ -o $@
m_dccchat.so: m_dccchat.o
	$(CXX) $(LDFLAGS) $^ -o $@
m_admin.so: m_admin.o
	$(CXX) $(LDFLAGS) $^ -o $@
m_admin_loadmod.so: m_admin_loadmod.o
	$(CXX) $(LDFLAGS) $^ -o $@
m_admin_unloadmod.so: m_admin_unloadmod.o
	$(CXX) $(LDFLAGS) $^ -o $@
m_admin_rehash.so: m_admin_rehash.o
	$(CXX) $(LDFLAGS) $^ -o $@
 
# Check for dependencies and recompile if required
# Implicitly takes the cpp as a dependency
# This line only makes sure the makeinclude.h file is checked
m_ctcppingreply.o: $(DEPS)
m_ctcpversionreply.o: $(DEPS)
m_ctcptimereply.o: $(DEPS)
m_dccchat.o: $(DEPS)
m_admin.o: $(DEPS)
m_admin_loadmod.o: $(DEPS)
m_admin_unloadmod.o: $(DEPS)
m_admin_rehash.o: $(DEPS)
 
.PHONY: clean
 
clean:
	rm -f *.o *.so