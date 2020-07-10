all: zenlisp

test: zenlisp
	./zenlisp --test

CFLAGS=-Wall -Wpedantic -g

zenlisp: src/*.c src/*.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o zenlisp src/*.c

clean:
	rm -rf zenlisp *.dSYM
