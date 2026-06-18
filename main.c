#include "chess.h"

int main(void) {
    
    Game game = new_game();

    do_move(&game, (Move){
        (P){4, 1}, (P){4, 3}
    });

    do_move(&game, (Move){
        (P){4, 6}, (P){4, 4}
    });

    do_move(&game, (Move){
        (P){6, 0}, (P){5, 2}
    });

    draw_game_small(&game);
    printf("\n");

    printf("sizeof P:     %lu\n", sizeof(P));
    printf("sizeof Move:  %lu\n", sizeof(Move));
    printf("sizeof Piece: %lu\n", sizeof(Piece));
    printf("sizeof Game:  %lu\n", sizeof(Game));
    printf("sizeof Board: %lu\n", sizeof(Board));

    printf("\n");

    calculate_possible_moves(&game);
    printf("\n");
    printf("%d\n", game.amount_of_possible_moves);
    printf("\n");

    for (U16 i = 0; i < game.amount_of_moves; i++) {
        printf("(%d, %d) -> (%d, %d)\n",
            game.moves[i].start.x,
            game.moves[i].start.y,
            game.moves[i].end.x,
            game.moves[i].end.y
        );
    }
}


