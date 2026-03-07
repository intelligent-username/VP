CC = gcc
CFLAGS = -Wall -D_REENTRANT \
         -Isrc/core \
         -Isrc/entities \
         -Isrc/use_cases \
         -Isrc/interface_adapters \
         -Isrc/frameworks \
         -Isrc/ipc \
         $(shell pkg-config --cflags gtk+-3.0)
LDFLAGS = $(shell pkg-config --cflags --libs libavcodec libavformat libavutil libswscale libswresample sdl2 gtk+-3.0) -lm -pthread

SRC = $(wildcard src/*.c \
                 src/core/*.c \
                 src/entities/*.c \
                 src/frameworks/*.c \
                 src/interface_adapters/*.c \
                 src/ipc/*.c \
                 src/use_cases/*.c)

OBJ = $(SRC:.c=.o)
TOTAL := $(words $(SRC))
CNT_FILE := .build_count

.PHONY: all run clean

all: vp

vp: $(OBJ)
	@printf "\n\e[1;33m[LINK]\e[0m Linking executable \e[1;32mvp\e[0m... "
	@$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)
	@rm -f $(CNT_FILE)
	@echo "DONE"

%.o: %.c
	@if [ ! -f $(CNT_FILE) ]; then echo 0 > $(CNT_FILE); fi
	@N=$$(($$(cat $(CNT_FILE)) + 1)); echo $$N > $(CNT_FILE); \
	 printf "\r\e[K\e[1;32m[%2d/$(TOTAL)]\e[0m Building \e[1;34m%-45s\e[0m" $$N "$<"
	@$(CC) -c -o $@ $< $(CFLAGS)

run: all
	./vp

clean:
	@echo "Cleaning up..."
	@find src -name '*.o' -delete 2>/dev/null || true
	@rm -f vp $(CNT_FILE)
