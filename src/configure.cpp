#include <iostream>
#include <fstream>
#include <vector>
#include <dirent.h>

int main(int argc, char** argv) {
	std::string mainMakefile = "default:\n\tmake -C src/\n\tmv src/robobo .\n\tmv src/modules/*.so modules\n\ndebug:\n\tmake debug -C src/\n\tmv src/robobo .\n\tvm src/modules/*.so modules\n\nclean:\n\tmake clean -C src/\n\trm -f robobo\n\trm -f modules/*\n\t@echo \"Clean has been made.\"";
	std::string moduleMakefileBegin = "CXX = g++\nCXXFLAGS=-fPIC -Wall -pipe -ansi $(DEBUG)\nLDFLAGS=-shared\nDEPS=modinclude.h ../main.h ../connection.h ../modules.h ../config.cpp ../modules.cpp ../modinterface.cpp\n\nall: ";
	std::string moduleMakefileEnd = ".PHONY: clean\n\nclean:\n\trm -f *.o *.so";
	std::ofstream makeOut;
	std::string dir = argv[0];
	dir = dir.substr(0, dir.find_last_of('/'));
	std::string moddir = dir + "/src/modules";
	std::string makedir = dir + "/Makefile";
	std::string modmakedir = dir + "/src/modules/Makefile";
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
		makeOut << moduleFiles[i] << ".so ";
	makeOut << "\n\n";
	for (unsigned int i = 0; i < moduleFiles.size(); i++)
		makeOut << moduleFiles[i] << ".so: " << moduleFiles[i] << ".o\n\t$(CXX) $(LDFLAGS) " << moduleFiles[i] << ".o -o $@\n";
	for (unsigned int i = 0; i < moduleFiles.size(); i++)
		makeOut << moduleFiles[i] << ".o: $(DEPS)\n";
	makeOut << moduleMakefileEnd;
	makeOut.close();
}