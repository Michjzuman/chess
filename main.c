#include "chess.h"


U8 human(const Game *game) {
    return 0;
}


U8 jonkler(const Game *game) {
    return rand() % game->amount_of_legal_moves;
}


U8 preprogrammed_bot(const Game *game) {
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
    play(preprogrammed_bot, preprogrammed_bot);
}
