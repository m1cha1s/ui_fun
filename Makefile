CC := cc
CFLAGS := $(shell pkg-config --cflags raylib)
LDFLAGS := $(shell pkg-config --libs raylib)

.PHONY: all clean run

all: main

clean:
	rm -f main

run: main
	./main

main: main.c
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)