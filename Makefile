CC := cc
CFLAGS := -Wall -ansi -pedantic
D_BASE16_BUFSIZ :=
TARGET := bin/base16

ifdef BASE16_BUFSIZ
D_BASE16_BUFSIZ := -DBASE16_BUFSIZ=$(BASE16_BUFSIZ)
endif

$(TARGET): bin/main.o bin/base16.o
	$(CC) -o $@ bin/main.o bin/base16.o

bin/main.o: src/main.c
	$(CC) $(CFLAGS) -c src/main.c -o $@

bin/base16.o: src/base16.h src/base16.c
	$(CC) $(CFLAGS) -c src/base16.c -o $@ $(D_BASE16_BUFSIZ)

clean:
	rm -f bin/*.o $(TARGET) 2>&1 > /dev/null

