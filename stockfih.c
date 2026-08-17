#include "chess.h"

enum CompactGameResult {
    UNDEFINED, DRAW,
    WHITE_WON, BLACK_WON
};

typedef struct {
    U16 len;
    U8 *moves;
    U8 result;
} CompactGame;

CompactGame compact_game(const Game *game) {
    CompactGame compact;
    compact.len = game->amount_of_moves;
    compact.moves =  malloc(compact.len);

    Game test_game = new_game();

    for (U8 i = 0; i < game->amount_of_moves; i++) {
        char *notation = game->moves[i];
        
        bool legal = false;
        for (U8 lm = 0; lm < game->amount_of_legal_moves; lm++) {
            if (strcmp(game->legal_moves[lm].notation, notation) == 0) {
                raw_move(&test_game, game->legal_moves[i]);
                legal = true;
            }
        }
        
        if (!legal) {
            close_game(&test_game);
            break;
        };

        if (
            test_game.amount_of_legal_moves <= 0
            && test_game.check
        ) {
            close_game(&test_game);
            compact.result = (
                test_game.turn == WHITE ?
                WHITE_WON : BLACK_WON
            );
            break;
        }
        if (test_game.draw) {
            close_game(&test_game);
            compact.result = DRAW;
            break;
        }

        test_game.turn = 1 - test_game.turn;
    }

    close_game(&test_game);
    return compact;
}

U8 stockfih(const Game *game, U0 *args) {



    return 0;
}