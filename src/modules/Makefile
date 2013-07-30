OBJECTS=$(shell for source in $(basename $(wildcard *.cpp)); do printf " ../../modules/$$source.so"; done)

.DEFAULT: default

.PHONY: default

default: ${OBJECTS}

clean:
	@rm -f modules.mk

include modules.mk