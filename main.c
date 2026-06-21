#include "chess.h"



int main_old(void) {
    Game game = new_game();

    while (true) {

        draw_game(&game);
        
        printf("%s\n", game.check ? "check" : "not check");
        printf("\n");
        
        printf("%d\n", game.amount_of_legal_moves);
        printf("\n");
        
        
        for (U16 i = 0; i < game.amount_of_legal_moves; i++) {
            printf("(%d, %d) -> (%d, %d) | %s\n",
                game.legal_moves[i].start.x,
                game.legal_moves[i].start.y,
                game.legal_moves[i].end.x,
                game.legal_moves[i].end.y,
                game.legal_moves[i].notation
            );
        }
        
        printf("\nyour move: ");
        char selected_move[50];

        while (true) {
            if (fgets(selected_move, sizeof(selected_move), stdin)) {
                if (do_move(&game, selected_move)) break;
            }

            printf("move is illegal. try again: ");
        };
        
    }

    return 0;
}

int main(void) {
    Game game = new_game();

    draw_game(&game);
    
    printf("%s\n", game.check ? "check" : "not check");
    printf("\n");
    
    printf("%d\n", game.amount_of_legal_moves);
    printf("\n");
    
    
    for (U16 i = 0; i < game.amount_of_legal_moves; i++) {
        printf("(%d, %d) -> (%d, %d) | %s\n",
            game.legal_moves[i].start.x,
            game.legal_moves[i].start.y,
            game.legal_moves[i].end.x,
            game.legal_moves[i].end.y,
            game.legal_moves[i].notation
        );
    }

    bool was_legal = do_move(&game, "e4");

    draw_game(&game);

    return 0;
}


