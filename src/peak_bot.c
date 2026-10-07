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

typedef union {
    struct {
        U8 move;
        U8 result;
        U16 count;
    };
    U0 *ptr;
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

U0 log_reasoning(const Game *game, Result *results, Result final) {
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
    U8 f = final.move;
    fprintf(
        file, "\n--> %u. [%s] %s in %u\n",
        f, game->legal_moves[f].notation,
        results[f].result == 0 ? "?" : (
            results[f].result == game->turn + 1 ? "winning" : "losing"
        ),
        results[f].count
    );
    fclose(file);
}

typedef struct {
    const Game *game;
    Status *status;
    U16 depth, max_depth;
    bool visualize;
} RecursionArgs;

U0 *peak_bot_recursion(U0 *pargs) {
    RecursionArgs *args = pargs;
    const Game *game = args->game;
    Status *status = args->status;
    U16 depth = args->depth;
    U16 max_depth = args->max_depth;
    bool visualize = args->visualize;

    if (game->amount_of_legal_moves <= 0 && game->check) {
        return (Result){
            .count = depth,
            .result = game->turn == WHITE ? 2 : 1
        }.ptr;
    } else if (game->draw || (max_depth != 0 && depth >= max_depth)) {
        return (Result){.count = depth, .result = 0}.ptr;
    }
    
    status->layers[depth].max = game->amount_of_legal_moves;
    status->depth = depth;
    Result *results = malloc(game->amount_of_legal_moves * sizeof(Result));
    if (!results) exit(1);
    for (U8 i = 0; i < game->amount_of_legal_moves; i++) {
        status->depth = depth;
        status->layers[depth].progress = i;

        Game test_game = copy_game(game);

        if (!do_move(&test_game, test_game.legal_moves[i].notation)) exit(1);
        if (visualize) tui(&test_game, false);

        Result result;
        result.ptr = peak_bot_recursion(&(RecursionArgs){
            .game = &test_game,
            .status = status,
            .depth = depth + 1,
            .max_depth = max_depth,
            .visualize = visualize
        });
        result.move = i;

        results[i] = result;
        close_game(&test_game);
    }
    U8 default_result = jonkler(game, NULL);
    Result final = results[default_result];
    for (U8 i = 0; i < game->amount_of_legal_moves; i++) {
        if (i != default_result) {
            Result this = results[i];
            bool this_wins = results[i].result == game->turn + 1;
            bool this_draws = results[i].result == 0;
            bool this_loses = !this_wins && !this_draws;
            bool winning = final.result == game->turn + 1;
            bool drawing = final.result == 0;
            bool losing = !winning && !drawing;
            if (
                (this_wins && (!winning || this.count < final.count)) ||
                (this_draws && (losing || (!winning && this.count > final.count))) ||
                (this_loses && losing && this.count > final.count)
            ) final = this;
        }
    }
    if (depth == 0) log_reasoning(game, results, final);
    free(results);
    status->count++;
    return final.ptr;
}

U8 peak_bot(const Game *game, U0 *max_depth) {
    Result result;
    result.ptr = peak_bot_recursion(&(RecursionArgs){
        .game = game,
        .status = &(Status){0},
        .depth = 0,
        .max_depth = (uintptr_t)max_depth,
        .visualize = true
    });
    return result.move;
}

