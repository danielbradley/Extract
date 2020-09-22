arch    := $(shell uname)
quasi   := libexec/quasi/bin/$(arch)/quasi
curl    := curl-7.72.0
base    := $(shell pwd)
curldir := $(TMPDIR)/extract

all: cc md

quasi: $(quasi)
	quasi -f source/c source/mt/*.txt

$(quasi):
	make -C libexec/quasi

curl: $(curldir)/$(curl) $(curldir)/$(curl)/config.status $(curldir)/$(curl)/lib/.libs/libcurl.a

$(curldir)/$(curl):
	mkdir $(curldir)
	cd $(curldir); tar jxvf "$(base)/dep/curl-7.72.0.tar.bz2"

$(curldir)/$(curl)/config.status:
	cd $(curldir)/$(curl); ./configure --disable-shared --enable-static

$(curldir)/$(curl)/lib/.libs/libcurl.a:
	cd $(curldir)/$(curl); make

cc: quasi curl
	mkdir  -p bin/$(arch)
	gcc    $(curldir)/$(curl)/lib/.libs/libcurl.a -lldap -lz -o bin/$(arch)/extract       source/c/main.c
	gcc -g $(curldir)/$(curl)/lib/.libs/libcurl.a -lldap -lz -o bin/$(arch)/extract-debug source/c/main.c

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
	rm -rf bin dep/curl-7.72.0

