#include <iostream>
#include <fstream>
#include <vector>
#include <dirent.h>

int main(int argc, char** argv) {
	std::string mainMakefile = "CC = g++\nLIBS = -ldl -lpthread\nLDFLAGS=$(LIBS) -rdynamic\nCXXFLAGS=-Wall -pthread $(DEBUG)\nSOURCES=modinterface.cpp server.cpp channel.cpp config.cpp modules.cpp server.cpp socket.cpp user.cpp\n\ndefault: makedepend robobo modules\n\n";
	mainMakefile += "makedepend:\n\tfor i in *.cpp; do for inc in  $(awk '/^#include \"/ {print $2}' $i | sed 's/\"//g'); do echo \"${i%.cpp}.o: $inc\"; done; done\n\ndebug:\n\tDEBUG=\"-g -O0\" make -C .\n\nrobobo: $(SOURCES:.cpp=.o)\n\n.PHONY: modules clean\n\nmodules:\n\tmake -C modules/\n\nclean:\n\tmake -C modules/ clean\n\trm -f robobo\n\trm -f $(SOURCES:.cpp=.o)";
	std::string moduleMakefileBegin = "CXX = g++\nCXXFLAGS=-fPIC -Wall -pipe -ansi -pedantic-errors $(DEBUG)\nLDFLAGS=-shared\nDEPS=modinclude.h ../main.h ../connection.h ../modules.h ../config.cpp ../modules.cpp ../modinterface.cpp\n\nall: modules\n\nmodules:";
	std::string moduleMakefileEnd = ".PHONY: clean\n\nclean:\n\trm -f *.o *.so";
	std::ofstream makeOut;
	std::string dir = argv[0];
	dir = dir.substr(0, dir.find_last_of('/'));
	std::string moddir = dir + "/modules";
	std::string makedir = dir + "/Makefile";
	std::string modmakedir = dir + "/modules/Makefile";
	makeOut.open(makedir.c_str(), std::ios::out | std::ios::trunc);
	makeOut << mainMakefile;
	makeOut.close();
	DIR* modulesDir = opendir(moddir.c_str());
	std::vector<std::string> moduleFiles;
	dirent* moduleFile;
	while (moduleFile = readdir(modulesDir)) {
		std::string moduleName = moduleFile->d_name;
		if (moduleName.size() > 4) {
			if (moduleName.substr(moduleName.size() - 4) == ".cpp") {
				moduleFiles.push_back(moduleName.substr(0, moduleName.size() - 4));
				std::cout << "Found module: " << moduleName << std::endl;
			}
		}
	}
	makeOut.open(modmakedir.c_str(), std::ios::out | std::ios::trunc);
	makeOut << moduleMakefileBegin;
	for (unsigned int i = 0; i < moduleFiles.size(); i++)
		makeOut << moduleFiles[i] << ".so: " << moduleFiles[i] << ".o\n\t$(CXX) $(LDFLAGS) $^ -o $@\n";
	for (unsigned int i = 0; i < moduleFiles.size(); i++)
		makeOut << moduleFiles[i] << ".o: $(DEPS)\n";
	makeOut << moduleMakefileEnd;
	makeOut.close();
}