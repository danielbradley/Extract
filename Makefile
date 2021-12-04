arch    := $(shell uname)
quasi   := libexec/quasi/bin/$(arch)/quasi
curl    := curl-7.80.0
base    := $(shell pwd)
curldir := tmp

ifeq ($(arch),Darwin)
	FRAMEWORKS=-framework CoreFoundation -framework SystemConfiguration
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

curl: $(curldir)/$(curl).tar.bz2 $(curldir)/$(curl) $(curldir)/$(curl)/config.status $(curldir)/$(curl)/lib/.libs/libcurl.a

$(curldir)/$(curl).tar.bz2:
	mkdir -p $(curldir)
	curl https://curl.se/download/$(curl).tar.bz2 --output $(curldir)/$(curl).tar.bz2

$(curldir)/$(curl):
	mkdir -p $(curldir)
	cd $(curldir); tar jxvf $(curl).tar.bz2

$(curldir)/$(curl)/config.status:
	cd $(curldir)/$(curl); ./configure --disable-shared --enable-static --without-ldap --without-brotli --with-nss

$(curldir)/$(curl)/lib/.libs/libcurl.a:
	cd $(curldir)/$(curl); make

cc: quasi curl
	mkdir  -p bin/$(arch)
	gcc $(FRAMEWORKS) -pthread    source/c/main.c $(curldir)/$(curl)/lib/.libs/libcurl.a $(LDLIBS) -I$(curldir)/$(curl)/include -o bin/$(arch)/extract
	gcc $(FRAMEWORKS) -pthread -g source/c/main.c $(curldir)/$(curl)/lib/.libs/libcurl.a $(LDLIBS) -I$(curldir)/$(curl)/include -o bin/$(arch)/extract-debug

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
	rm -rf bin tmp

