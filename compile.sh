#/bin/sh

# A simple compile script for until I get to the point where I make a
# real one.  This may output warnings.  I'm trying to not have warnings,
# so get over it.  If there are warnings it means I haven't solved them
# yet.

if [ "$1" = "debug" ]
then
	echo Compiling with debug symbols.
	g++ -ansi -pedantic-errors -Wall -g -O0 -fno-inline robobo.cpp -o robobo
else
	echo Compiling normally.
	g++ -ansi -pedantic-errors -Wall -ldl robobo.cpp -o robobo
fi