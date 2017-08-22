all: quasi

pull:
	git pull

quasi:
	quasi -f output README.md

pandoc:
	pandoc -o README.html README.md

push:
	git add .
	git commit -m "Changes."
	git push

clean:
	rm -rf output
