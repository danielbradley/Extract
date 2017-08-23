all: quasi

pull:
	git pull

quasi:
	quasi -f _gen README.md

pandoc:
	pandoc -o _gen/README.html README.md

push:
	git add .
	git commit -m "Changes."
	git push

clean:
	rm -rf _gen
