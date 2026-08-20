#include "chess.h"
#include "tui.h"

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
    compact.moves =  malloc(compact.len * sizeof(U8));

    Game test_game = new_game();

    for (U8 i = 0; i < game->amount_of_moves; i++) {
        char *notation = game->moves[i];

        tui_small(&test_game, true);
        
        for (U8 t = 0; t < test_game.amount_of_legal_moves; t++) {
            printf("%s ", test_game.legal_moves[t].notation);
        }
        printf("\n");
        printf("{ %s }\n", notation);

        bool legal = false;
        for (U8 lm = 0; lm < test_game.amount_of_legal_moves; lm++) {
            if (strcmp(test_game.legal_moves[lm].notation, notation) == 0) {
                compact.moves[i] = lm;
                raw_move(&test_game, test_game.legal_moves[lm]);
                legal = true;
            }
        }

        if (!legal) {
            // for some reason this always occurs if in the last move a piece was taken.
            printf("illegal: %u\n", i);
            printf("notation: %s\n", notation);
            exit(1);
            break;
        };

        if (
            test_game.amount_of_legal_moves <= 0
            && test_game.check
        ) {
            compact.result = (
                test_game.turn == WHITE ?
                WHITE_WON : BLACK_WON
            );
            break;
        }
        if (test_game.draw) {
            compact.result = DRAW;
            break;
        }
    }

    close_game(&test_game);

    for (U16 i = 0; i < compact.len; i++) {
        printf("[%d] ", compact.moves[i]);
    }
    printf("\n");
    exit(0);
    
    return compact;
}

U0 write_compactgame_to_file(CompactGame *compact) {

}

U8 stockfih(const Game *game, U0 *args) {

    if (game->amount_of_moves > 20) {
        CompactGame compact = compact_game(game);
        free(compact.moves);
    }

    return 0;
}
