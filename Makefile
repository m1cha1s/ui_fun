CC := cc
CFLAGS := $(shell pkg-config --cflags raylib) -g -fsanitize=address
LDFLAGS := $(shell pkg-config --libs raylib) -g -fsanitize=address

.PHONY: all clean run

all: main music_player

clean:
	rm -f main

run: main music_player
	./music_player

main: main.c
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

music_player: music_player.c
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)
