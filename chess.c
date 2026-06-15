#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define S 8
#define a {1, 2, 3}

const char *board[8][8] = {
    {"R", "N", "B", "Q", "K", "B", "N", "R"},
    {"P", "P", "P", "P", "P", "P", "P", "P"},
    {".", ".", ".", ".", ".", ".", ".", "."},
    {".", ".", ".", ".", ".", ".", ".", "."},
    {".", ".", ".", ".", ".", ".", ".", "."},
    {".", ".", ".", ".", ".", ".", ".", "."},
    {"p", "p", "p", "p", "p", "p", "p", "p"},
    {"r", "n", "b", "q", "k", "b", "n", "r"},
};

struct Move {
    int fromX;
    int fromY;
    int toX;
    int toY;
};

void draw() {
    for (int y = 7; y >= 0; y--) {
        for (int x = 0; x < 8; x++) {
            printf(" %s", board[y][x]);
        }
        printf("\n");
    }
}

struct Move possibleMoves() {
    struct Move result[];
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (board[y][x] == 'P') {
                result = (struct Move){
                    x, y, x, y - 1
                };
            }
        }
    }
    return result;
}

void doMove(struct Move move) {
    board[move.toY][move.toX] = board[move.fromY][move.fromX];
    board[move.fromY][move.fromX] = ".";
}

int main() {

    doMove((struct Move){4, 1, 4, 3});
    doMove((struct Move){4, 6, 4, 5});

    draw();
}

