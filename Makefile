arch    := $(shell uname)
cpu     := $(shell uname -m)
quasi   := libexec/quasi/_bin/$(arch)/quasi
curl    := curl-7.80.0
base    := $(shell pwd)
bin     := _bin

ifeq ($(arch),Darwin)
	FRAMEWORKS=-framework CoreFoundation -framework SystemConfiguration -Wdeprecated-declarations
	LDLIBS=-lldap -lz
else
	FRAMEWORKS=
	LDLIBS=-lz
endif


all: cc md

quasi: $(quasi)
	$(quasi) -f source/c source/mt/*.txt

$(quasi):
	make -C libexec/quasi

cc: quasi
	mkdir  -p $(bin)/$(arch)-$(cpu)
	gcc $(FRAMEWORKS)    source/c/main.c -o $(bin)/$(arch)-$(cpu)/extract
	gcc $(FRAMEWORKS) -g source/c/main.c -o $(bin)/$(arch)-$(cpu)/extract-debug

md:
	cat source/mt/*.txt | sed 's|^\.\.\.|####|g' \
                            | sed 's|^\.\.|###|g'    \
                            | sed 's|^\.|##|g'       \
                            | sed 's|^--||g'         \
                            | sed 's|^-|#|g'         \
                            | sed 's|^\~!|```!|g'    \
                            | sed 's|^\~|```|g'      \
                            | sed 's|\~$$||g'        \
                            | sed 's|"https://www.quasi-literateprogramming.org"|[https://www.quasi-literateprogramming.org]|g' \
                            > README.md

doc: pandoc max2html

pandoc:
	pandoc -o doc/README.html README.md

maxtext:
	max2html --out doc/extract --style share/css/style.css source/mt/*.txt

clean:
	make -C libexec/quasi clean
	rm -rf $(bin)

