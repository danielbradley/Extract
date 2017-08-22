all: quasi

pull:
	git pull

quasi:
	quasi -f output README.md

clean:
	rm -rf output
