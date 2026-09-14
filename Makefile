# chess

ALL = src/*.*
SRC = src/*.c
FLAGS = -I src -lm

all: chess

chess: $(ALL) main.c
	clang $(SRC) main.c $(FLAGS) -o chess

test: $(ALL) test.c
	clang $(SRC) test.c $(FLAGS) -o test
