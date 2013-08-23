OBJECTS=$(shell for source in $(basename $(wildcard *.cpp)); do printf " ../../modules/$$source.so"; done; for sourcedir in $(basename $(wildcard */)); do printf " ../../modules/$${sourcedir%?}.so"; done)

.DEFAULT: default

.PHONY: default clean

default: ${OBJECTS}

clean:
	rm -f *.o
	$(shell for dir in $(basename $(wildcard */)); do rm -f $$dir*.o; done)

include modules.mk