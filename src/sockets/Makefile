OBJECTS=$(shell for source in $(basename $(wildcard *.cpp)); do printf " ../../sockets/$$source.so"; done; for sourcedir in $(basename $(wildcard */)); do printf " ../../sockets/$${sourcedir%?}.so"; done)

.DEFAULT: default

.PHONY: default

default: ${OBJECTS}

include modules.mk