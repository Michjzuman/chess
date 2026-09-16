#include "chess.h"
#include "tui.h"
#include "bots.h"

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
#define min(a, b) (((a) < (b)) ? (a) : (b))

double calculate_progress(Status *status) {
    double result = 0.0f;
    double max = 1.0f;
    for (U16 i = 0; i <= status->depth; i++) {
        max *= (double)status->layers[i].max;
        double progress = (double)status->layers[i].progress / max;
        result += progress;
    }
    return result;
}

U0 status_bar(double p, U8 w) {
    printf("[");
    for (U16 x = 0; x < w; x++) {
        printf((p > (double)x / w) ? "=" : " ");
    }
    printf("]");
}

U0 log_progress(Status *status) {
    static U16 previous_log_lines = 0;
    printf("\033[%uF", previous_log_lines);
    status_bar(calculate_progress(status), 31);
    printf("\n");
    previous_log_lines = 1;
}

U0 log_status(Status *status) {
    static U16 previous_log_lines = 0;

    U16 shown_log_lines = 30;
    U16 first_line = (
        status->depth >= shown_log_lines ?
        status->depth - shown_log_lines + 1 : 0
    );

    if (previous_log_lines) printf("\033[%uF", previous_log_lines);

    U16 h = 0;
    for (U16 i = first_line; i <= status->depth; i++) {
        double p = (
            (double)status->layers[i].progress / (double)status->layers[i].max
        );
        printf("%d. ", i);
        status_bar(p, 40);
        printf(
            " %d%% (%u/%u)   \n",
            (int){p * 100.0f},
            status->layers[i].progress, status->layers[i].max
        );
        h++;
    }
    for (U8 i = 0; i < 67; i++) printf("-");
    printf("\ntotal: %llu (%.16f%%)", status->count, calculate_progress(status) * 100.0f);
    for (U8 i = 0; i < 40; i++) printf(" ");
    printf("\n");
    for (U8 i = 0; i < 67; i++) printf(" ");
    printf("\n");
    previous_log_lines = h + 3;
}

typedef struct {
    const Game *game;
    Status *status;
    U16 depth, max_depth;
    bool log, verbose;
} RecursionArgs;

Result peak_bot_recursion(
    const Game *game, Status *status, U16 depth, U16 max_depth,
    bool log, bool verbose
) {
    //if (strcmp(game->moves[0], "f4") == 0) usleep(100000);

    if (game->amount_of_legal_moves <= 0 && game->check) {
        return (Result){
            .count = depth,
            .result = game->turn == WHITE ? 2 : 1
        };
    } else if (game->draw || (max_depth != 0 && depth >= max_depth)) {
        return (Result){.count = depth, .result = 0};
    }
    
    status->layers[depth].max = game->amount_of_legal_moves;
    status->depth = depth;
    Result *results = malloc(game->amount_of_legal_moves * sizeof(Result));
    if (!results) exit(1);
    for (U8 i = 0; i < game->amount_of_legal_moves; i++) {
        status->depth = depth;
        status->layers[depth].progress = i;

        if (log && status->count % 1000 == 0) {
            if (verbose) {
                if (status->count % 1 == 0) {
                    log_status(status);
                }
            } else if (status->count % 1 == 0) {
                log_progress(status);
            }
        }

        Game test_game = copy_game(game);

        if (!do_move(&test_game, test_game.legal_moves[i].notation)) exit(1);
        //tui(&test_game, false);

        Result result = peak_bot_recursion(
            &test_game, status, depth + 1, max_depth,
            log > 0, verbose
        );
        result.move = i;

        results[i] = result;
        close_game(&test_game);
    }
    U8 default_result = jonkler(game, NULL);
    Result final = results[default_result];
    for (U8 i = 0; i < game->amount_of_legal_moves; i++) {
        if (i != default_result) {
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
    }
    if (depth == 0) {
        FILE *file = fopen("peak_bot_reasoning.log", "w");
        for (U8 i = 0; i < game->amount_of_legal_moves; i++) {
            fprintf(
                file, "%u. [%s] %s in %u\n",
                i, game->legal_moves[i].notation,
                results[i].result == 0 ? "?" : (
                    results[i].result == game->turn + 1 ? "winning" : "losing"
                ),
                results[i].count
            );
        }
        fclose(file);
    }
    free(results);
    status->count++;
    return final;
}

U8 peak_bot(const Game *game, U0 *max_depth) {
    return peak_bot_recursion(
        game, &(Status){0}, 0, (uintptr_t)max_depth,
        true, (uintptr_t)max_depth > 3
    ).move;
}
