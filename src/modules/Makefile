CXX = g++
CXXFLAGS=-fPIC -Wall -pipe -ansi $(DEBUG)
LDFLAGS=-shared
DEPS=modinclude.h ../main.h ../connection.h ../modules.h ../config.cpp ../modules.cpp ../modinterface.cpp

all: m_admin_rehash.so m_admin_say.so m_admin_die.so m_admin_unloadmod.so m_admin.so m_admin_channels.so m_dccchat.so m_ctcppingreply.so m_ctcpversionreply.so m_admin_connect.so m_admin_raw.so m_ctcptimereply.so m_admin_loadmod.so m_setx.so m_services_identify.so 

m_admin_rehash.so: m_admin_rehash.o
	$(CXX) $(LDFLAGS) m_admin_rehash.o -o $@
m_admin_say.so: m_admin_say.o
	$(CXX) $(LDFLAGS) m_admin_say.o -o $@
m_admin_die.so: m_admin_die.o
	$(CXX) $(LDFLAGS) m_admin_die.o -o $@
m_admin_unloadmod.so: m_admin_unloadmod.o
	$(CXX) $(LDFLAGS) m_admin_unloadmod.o -o $@
m_admin.so: m_admin.o
	$(CXX) $(LDFLAGS) m_admin.o -o $@
m_admin_channels.so: m_admin_channels.o
	$(CXX) $(LDFLAGS) m_admin_channels.o -o $@
m_dccchat.so: m_dccchat.o
	$(CXX) $(LDFLAGS) m_dccchat.o -o $@
m_ctcppingreply.so: m_ctcppingreply.o
	$(CXX) $(LDFLAGS) m_ctcppingreply.o -o $@
m_ctcpversionreply.so: m_ctcpversionreply.o
	$(CXX) $(LDFLAGS) m_ctcpversionreply.o -o $@
m_admin_connect.so: m_admin_connect.o
	$(CXX) $(LDFLAGS) m_admin_connect.o -o $@
m_admin_raw.so: m_admin_raw.o
	$(CXX) $(LDFLAGS) m_admin_raw.o -o $@
m_ctcptimereply.so: m_ctcptimereply.o
	$(CXX) $(LDFLAGS) m_ctcptimereply.o -o $@
m_admin_loadmod.so: m_admin_loadmod.o
	$(CXX) $(LDFLAGS) m_admin_loadmod.o -o $@
m_setx.so: m_setx.o
	$(CXX) $(LDFLAGS) m_setx.o -o $@
m_services_identify.so: m_services_identify.o
	$(CXX) $(LDFLAGS) m_services_identify.o -o $@
m_admin_rehash.o: $(DEPS)
m_admin_say.o: $(DEPS)
m_admin_die.o: $(DEPS)
m_admin_unloadmod.o: $(DEPS)
m_admin.o: $(DEPS)
m_admin_channels.o: $(DEPS)
m_dccchat.o: $(DEPS)
m_ctcppingreply.o: $(DEPS)
m_ctcpversionreply.o: $(DEPS)
m_admin_connect.o: $(DEPS)
m_admin_raw.o: $(DEPS)
m_ctcptimereply.o: $(DEPS)
m_admin_loadmod.o: $(DEPS)
m_setx.o: $(DEPS)
m_services_identify.o: $(DEPS)
.PHONY: clean

clean:
	rm -f *.o *.so