
all: bin/voip

bin/voip: bin/main.o
	gcc -lportaudio bin/main.o -o bin/voip 

bin/main.o: src/main.c
	gcc -c src/main.c -o bin/main.o
clean:
	rm -f bin/*.o bin/voip

