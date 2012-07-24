cd src/modules
for source in *.cpp; do 
	COMPILE_FLAGS=`grep '^s*// COMPILE_FLAGS ' $source | awk -F'[ ]' '{$1=$2=""; print $0}'`
	printf "`g++ -std=c++0x -E -MM $source -MT ../../modules/${source:0:${#source}-4}.so`\n\t@echo \"\e[1;32mBuild:\e[0m\t$source\"\n\t@g++ -std=c++0x -Wall -pedantic -pthread -rdynamic $DEBUG -pipe -fPIC -shared $source -ldl $COMPILE_FLAGS -c -o ../../modules/${source:0:${#source}-4}.so\n\n"
done
