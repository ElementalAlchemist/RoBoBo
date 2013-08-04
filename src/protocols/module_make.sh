cd src/protocols # This script is called from the root makefile
shopt -s nullglob
for source in *.cpp; do
	printf "`$CC $GENDEPS $source $GENDEPSTARGET ../../protocols/${source:0:${#source}-4}.so`\n\t@echo \"\e[1;32mBuild:\e[0m\t$source\"\n\t@$CC $CXXFLAGS $SOFLAGS $source `grep '^\s*// COMPILE_FLAGS ' $source | awk -F'[ ]' '{$1=$2=""; print $0}'` -c -o ../../protocols/${source:0:${#source}-4}.so\n\n"
done