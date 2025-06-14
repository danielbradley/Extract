arch=$(shell uname)

cc: _bin/$(arch)/quasi

_bin/$(arch)/quasi:
	mkdir -p _bin/$(arch)
	cc -o _bin/$(arch)/quasi source/c/quasi.c

clean:
	rm -rf _bin
