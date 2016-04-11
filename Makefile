CC := cc
CFLAGS := -Wall -ansi -pedantic
D_PARSER_BUFSIZ :=
TARGET := bin/xtool

ifdef PARSER_BUFSIZ
D_PARSER_BUFSIZ := -DPARSER_BUFSIZ=$(PARSER_BUFSIZ)
endif

$(TARGET): bin/main.o bin/parser.o
	$(CC) -o $@ bin/main.o bin/parser.o

bin/main.o: src/main.c
	$(CC) $(CFLAGS) -c src/main.c -o $@

bin/parser.o: src/parser.h src/parser.c
	$(CC) $(CFLAGS) -c src/parser.c -o $@ $(D_PARSER_BUFSIZ)

clean:
	rm -f bin/*.o $(TARGET) 2>&1 > /dev/null

