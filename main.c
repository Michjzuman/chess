#include "chess.h"
#include "tui.h"
#include "bots.h"

U8 testing_bot(const Game *game) {
    char *moves[] = {
        "e4", "e5",
        "Nf3", "d5",
        "d4", "Nc6",
        "Bb5"
    };

    if (
        game->amount_of_moves <
        sizeof(moves) / sizeof(char *)
    ) {
        for (
            U8 i = 0;
            i < game->amount_of_legal_moves;
            i++
        ) {
            if (
                strcmp(game->legal_moves[i].notation,
                moves[game->amount_of_moves]) == 0
            ) {
                return i;
            } 
        }
    }

    exit(0);
    return jonkler(game);
}

int main(void) {
    Color winner = play(draw_game, human, jonkler);
    printf("%s\n",
        winner == 0 ? "draw" :
        winner == 1 ? "green" : "blue"
    );
    printf("%lu\n", sizeof(Game) * 240);
    printf("%lu\n", sizeof(P));
    return 0;
}

