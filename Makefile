arch := $(shell uname)

all: quasi cc md

quasi:
	quasi -f source/c source/mt/*.txt

cc:
	mkdir -p bin/$(arch)
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

pull:
	git pull

pandoc:
	pandoc -o doc/README.html README.md

push:
	git push
