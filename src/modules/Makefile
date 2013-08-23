OBJECTS=$(shell for source in $(basename $(wildcard *.cpp)); do printf " ../../modules/$$source.so"; done; for sourcedir in $(basename $(wildcard */)); do printf " ../../modules/$${sourcedir%?}.so"; done)

.DEFAULT: default

.PHONY: default

default: ${OBJECTS}

include modules.mk