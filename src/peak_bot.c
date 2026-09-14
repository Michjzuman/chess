#include "chess.h"
#include "tui.h"

typedef struct {
    U8 progress, max;
} StatusLayer;

typedef struct {
    StatusLayer layers[65536];
    U16 depth;
    U64 count;
} Status;

typedef struct {
    U8 move;
    U8 result;
    U16 count;
} Result;

#define max(a, b) (((a) > (b)) ? (a) : (b))

double calculate_progress(Status *status) {
    double result = 0.0f;
    double max = 1.0f;
    for (U16 i = 0; i <= status->depth; i++) {
        max *= (double)status->layers[i].max;
        double progress = (double)status->layers[i].progress / max;
        result += progress;
    }
    return result * 100.0f;
}

U0 log_status(Status *status) {
    static U16 previous_log_lines = 0;

    U16 shown_log_lines = 40;
    U16 first_line = (
        status->depth >= shown_log_lines ?
        status->depth - shown_log_lines + 1 : 0
    );

    if (previous_log_lines) printf("\033[%uF", previous_log_lines);

    for (U16 i = first_line; i <= status->depth; i++) {
        double p = (
            (double)status->layers[i].progress / (double)status->layers[i].max
        );
        printf("%d. [", i);
        for (U16 x = 0; x < 50; x++) {
            printf((p > (double)x / 50.0f) ? "=" : " ");
        }
        printf(
            "] %d%% (%u/%u)   \n",
            (int){p * 100.0f},
            status->layers[i].progress, status->layers[i].max
        );
    }
    printf("total: %llu (%.16f%%)\n", status->count, calculate_progress(status));

    previous_log_lines = status->depth - first_line + 2;
}

U8 peak_bot_recursion(const Game *game, Status *status, U16 depth) {
    //usleep(100000);
    if (game->amount_of_legal_moves == 0) return 0;
    status->layers[depth].max = game->amount_of_legal_moves;
    status->depth = depth;
    Result *results = malloc(game->amount_of_legal_moves * sizeof(Result));
    if (!results) exit(1);
    for (U8 i = 0; i < game->amount_of_legal_moves; i++) {
        status->depth = depth;
        status->layers[depth].progress = i;

        log_status(status);

        Game test_game = copy_game(game);

        if (!do_move(&test_game, test_game.legal_moves[i].notation)) exit(1);
        //tui(&test_game, false);

        U8 result = 0;

        if (test_game.amount_of_legal_moves <= 0 && test_game.check) {
            result = test_game.turn == WHITE ? 2 : 1;
        } else if (!test_game.draw && test_game.amount_of_legal_moves > 0) while (true) {
            U8 move = peak_bot_recursion(&test_game, status, depth + 1);
            char *notation = test_game.legal_moves[move].notation;
            if (!do_move(&test_game, notation)) exit(1);
            if (test_game.amount_of_legal_moves <= 0 && test_game.check) {
                result = test_game.turn == WHITE ? 2 : 1;
                break;
            }
            if (test_game.draw) break;
        }

        results[i].move = i;
        results[i].result = result;
        results[i].count = test_game.amount_of_moves;
        close_game(&test_game);
    }
    Result final = results[0];
    for (U8 i = 1; i < game->amount_of_legal_moves; i++) {
        if (results[i].result == game->turn + 1) {
            if (
                final.result != game->turn + 1 ||
                final.count > results[i].count
            ) {
                final = results[i];
            }
        } else if (results[i].result == 0) {
            if (
                final.result != game->turn + 1 &&
                final.count < results[i].count
            ) {
                final = results[i];
            }
        } else {
            if (
                final.result != 0 &&
                final.result != game->turn + 1 &&
                final.count < results[i].count
            ) {
                final = results[i];
            }
        }
    }
    free(results);
    status->count++;
    return final.move;
}

U8 peak_bot(const Game *game, U0 *args) {
    return peak_bot_recursion(game, &(Status){0}, 0);
}