CC := cc
CFLAGS := -Wall -ansi -pedantic
TARGET := bin/xtool

$(TARGET): bin/main.o bin/parser.o
	$(CC) -o $@ bin/main.o bin/parser.o

bin/main.o: src/main.c
	$(CC) $(CFLAGS) -c src/main.c -o $@

bin/parser.o: src/parser.h src/parser.c
	$(CC) $(CFLAGS) -c src/parser.c -o $@

clean:
	rm -f bin/*.o $(TARGET) 2>&1 > /dev/null

