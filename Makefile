CC := cc
CFLAGS := $(shell pkg-config --cflags raylib) -g -fsanitize=address
LDFLAGS := $(shell pkg-config --libs raylib) -g -fsanitize=address

.PHONY: all clean run

all: main

clean:
	rm -f main

run: main
	./main

main: main.c
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)
