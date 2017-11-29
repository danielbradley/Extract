ARCH := $(shell uname)

release:
	mkdir -p bin/$(ARCH)
	gcc -o bin/$(ARCH)/extract source/src/extract.c

debug:
	mkdir -p bin/$(ARCH)
	gcc -g -o bin/$(ARCH)/extract-debug source/src/extract.c

