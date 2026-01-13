CC                = gcc

all: main armtohex armtole armtolle

main: main.c
	$(CC) $< -o $@

armtohex: armtohex.c
	$(CC) $< -o $@

armtole: armtole.c
	$(CC) $< -o $@

armtolle: armtolle.c
	$(CC) $< -o $@

clean:
	rm ./main
	rm ./armtohex
	rm ./armtole
	rm ./armtolle