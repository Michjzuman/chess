# chess

ALL = src/*.*
SRC = src/*.c
FLAGS = -I src -lm -pthread
PREFIX ?= /usr/local

all: chess

.PHONY: install debug test

install: chess
	@tmp="$(PREFIX)/bin/.chess.tmp.$$$$"; \
	cp chess "$$tmp"; \
	chmod 755 "$$tmp"; \
	mv -f "$$tmp" "$(PREFIX)/bin/chess"

chess: $(ALL) main.c
	clang $(SRC) main.c $(FLAGS) -o chess

debug: $(ALL) main.c
	clang $(SRC) main.c $(FLAGS) -o chess -g -O0

test: $(ALL) test.c
	clang $(SRC) test.c $(FLAGS) -o test -g -O0
