arch := $(shell uname)

all: bin

pull:
	git pull

quasi:
	quasi -f _gen README.md

bin: quasi
	mkdir -p extract/bin/$(arch)
	gcc -o extract/bin/$(arch)/extract _gen/main.c

pandoc:
	pandoc -o _gen/README.html README.md

push:
	git add .
	git commit -m "Changes."
	git push

clean:
	rm -rf _gen
