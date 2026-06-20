#include "chess.h"

int main(void) {
    
    Game game = new_game();

    do_move(&game, (Move){(P){4, 1}, (P){4, 3}});

    do_move(&game, (Move){(P){3, 6}, (P){3, 4}});

    do_move(&game, (Move){(P){6, 0}, (P){5, 2}});

    do_move(&game, (Move){(P){1, 7}, (P){2, 5}});
    //do_move(&game, (Move){(P){1, 7}, (P){0, 5}});

    do_move(&game, (Move){(P){5, 0}, (P){1, 4}});

    draw_game_small(&game);
    printf("\n");

    printf("sizeof P:     %lu\n", sizeof(P));
    printf("sizeof Move:  %lu\n", sizeof(Move));
    printf("sizeof Piece: %lu\n", sizeof(Piece));
    printf("sizeof Game:  %lu\n", sizeof(Game));

    printf("\n");
    
    printf("%d\n", game.amount_of_legal_moves);
    printf("\n");
    
    printf("%s\n", game.check ? "check" : "not check");
    printf("\n");

    for (U16 i = 0; i < game.amount_of_legal_moves; i++) {
        printf("(%d, %d) -> (%d, %d)\n",
            game.legal_moves[i].start.x,
            game.legal_moves[i].start.y,
            game.legal_moves[i].end.x,
            game.legal_moves[i].end.y
        );
    }
}


