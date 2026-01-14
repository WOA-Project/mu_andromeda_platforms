CC                = gcc

all: main armtohex armtole armtolle preproc preproc2

main: main.c
	$(CC) $< -o $@

armtohex: armtohex.c
	$(CC) $< -o $@

armtole: armtole.c
	$(CC) $< -o $@

armtolle: armtolle.c
	$(CC) $< -o $@

preproc: preproc.c
	$(CC) $< -o $@

preproc2: preproc2.c
	$(CC) $< -o $@

clean:
	rm ./main
	rm ./armtohex
	rm ./armtole
	rm ./armtolle
	rm ./preproc
	rm ./preproc2