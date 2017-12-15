arch := $(shell uname)
quasi=libexec/quasi/bin/$(arch)/quasi

all: quasi cc md

quasi: $(quasi)
	quasi -f source/c source/mt/*.txt

$(quasi):
	make -C libexec/quasi

cc:
	mkdir  -p bin/$(arch)
	gcc    -o bin/$(arch)/extract       source/c/main.c
	gcc -g -o bin/$(arch)/extract-debug source/c/main.c

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
	rm -rf bin

