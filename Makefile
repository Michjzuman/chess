# chess

ALL = src/*.*
SRC = src/*.c
FLAGS = -I src -lm -pthread

all: chess

chess: $(ALL) main.c
	clang $(SRC) main.c $(FLAGS) -o chess

debug: $(ALL) main.c
	clang $(SRC) main.c $(FLAGS) -o chess -g -O0

test: $(ALL) test.c
	clang $(SRC) test.c $(FLAGS) -o test -g -O0
