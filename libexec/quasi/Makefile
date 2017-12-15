arch=$(shell uname)

cc: bin/$(arch)/quasi

bin/$(arch)/quasi:
	mkdir -p bin/$(arch)
	cc -o bin/$(arch)/quasi source/c/quasi.c

clean:
	rm -r bin
