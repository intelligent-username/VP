# -j for Parallel compilation
# auto-detect CPU cores for maximum parallelism
MAKEFLAGS += -j

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

# Source Files & Build Configuration

SRC = $(wildcard src/*.c \
                 src/core/*.c \
                 src/entities/*.c \
                 src/frameworks/*.c \
                 src/interface_adapters/*.c \
                 src/ipc/*.c \
                 src/use_cases/*.c)

OBJ = $(patsubst src/%.c,build/obj/%.o,$(SRC))
TOTAL := $(words $(OBJ))

# Precompiled Headers (PCH)
# app_context.h includes heavy FFmpeg/SDL headers. 
# So if we compile once, reuse in all .c files

PRECOMP = src/core/app_context.h.gch
CNT_FILE = build/.build_count
LOCK_DIR = build/.build_lock
INIT_FILE = build/.build_init

.PHONY: all run clean rebuild

all: $(INIT_FILE) $(PRECOMP) vp

$(INIT_FILE):
	@mkdir -p build
	@echo 0 > $(CNT_FILE)
	@rm -rf $(LOCK_DIR)
	@touch $(INIT_FILE)

$(PRECOMP): src/core/app_context.h
	@printf "\e[1;33m[PRECOMP]\e[0m Compiling precompiled header... "
	@$(CC) -x c-header $(CFLAGS) -o $@ $<
	@echo "DONE"

# Last step: link everything into just the vp executable

vp: $(OBJ)
	@printf "\n\e[1;33m[LINK]\e[0m Linking executable \e[1;32mvp\e[0m... "
	@$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)
	@echo "DONE"

build/obj/%.o: src/%.c $(PRECOMP) $(INIT_FILE)
	@mkdir -p $(dir $@)
	@while ! mkdir $(LOCK_DIR) 2>/dev/null; do sleep 0.01; done; \
	 if [ ! -f $@ ]; then \
		 N=$$(cat $(CNT_FILE)); N=$$((N+1)); echo $$N > $(CNT_FILE); \
	 fi; rmdir $(LOCK_DIR); \
	 printf "\e[1;32m[%2d/$(TOTAL)]\e[0m Building \e[1;34m%s\e[0m\r" $$(cat $(CNT_FILE)) "$<"
	@$(CC) -c -o $@ $< $(CFLAGS)

run: all
	./vp

clean:
	@echo "Cleaning up..."
	@find src -name '*.o' -delete 2>/dev/null || true
	@rm -rf build
	@find src -name '*.gch' -delete 2>/dev/null || true
	@rm -f vp

rebuild: clean all
