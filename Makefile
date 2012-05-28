
CFLAGS := -g -std=gnu99

all: bin obj models bin/voip

.PHONY: models clean

bin:
	mkdir bin

obj:
	mkdir obj

models:
	chmod +x models/*.py

bin/voip: obj/voip.o obj/markov.o
	gcc $^ -lportaudio -o $@

obj/voip.o: src/voip.c src/markov.h
	gcc -c $(CFLAGS) $< -o $@

obj/markov.o: src/markov.c src/markov.h
	gcc -c $(CFLAGS) $< -o $@

clean:
	rm -f bin/* obj/*

