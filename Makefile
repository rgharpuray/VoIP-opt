
CFLAGS := -g -std=gnu99

all: bin obj bin/voip bin/main bin/paread bin/pawrite bin/pareadwrite bin/udptestclient bin/udptestserv

bin:
	mkdir bin

obj:
	mkdir obj

bin/%: obj/%.o
	gcc $< -lportaudio -o $@

obj/%.o: src/%.c
	gcc -c $(CFLAGS) $< -o $@


clean:
	rm -f bin/* obj/*

