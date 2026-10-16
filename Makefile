# chess

ALL = src/*.*
SRC = src/*.c
FLAGS = -I src -lm -pthread
RAYLIB_FLAGS = -D RAYLIB -lraylib -framework OpenGL \
	-framework Cocoa -framework IOKit -framework CoreVideo

PREFIX := /usr
ifeq ($(shell uname -s),Darwin)
    PREFIX := /usr/local
    RAYLIB_PREFIX ?= $(if $(filter arm64%,$(shell clang -print-target-triple)),/opt/homebrew,/usr/local)/opt/raylib
    RAYLIB_FLAGS += -I $(RAYLIB_PREFIX)/include -L $(RAYLIB_PREFIX)/lib
endif
ifeq ($(shell uname -s),Linux)
    FLAGS += -latomic
endif

all: chess

.PHONY: install debug test memvisualizer

install: chess
	@tmp="$(PREFIX)/bin/.chess.tmp.$$$$"; \
	cp chess "$$tmp"; \
	chmod 755 "$$tmp"; \
	mv -f "$$tmp" "$(PREFIX)/bin/chess"

chess: $(ALL) main.c
	clang $(SRC) main.c $(FLAGS) -o chess

memvisualizer: $(ALL)
	clang $(SRC) $(FLAGS) $(RAYLIB_FLAGS) -o vis -g -O0

debug: $(ALL) main.c
	make memvisualizer
	clang $(SRC) main.c $(FLAGS) -o chess -g -O0

test: $(ALL) test.c
	clang $(SRC) test.c $(FLAGS) -o test -g -O0
