#include "chess.h"
#include "tui.h"
#include "bots.h"

#include <pthread.h>
#include <stdatomic.h>

typedef atomic_int_least64_t atomic_U16;

enum {DRAW, WINNING, LOSING, UNKNOWN};
#define OUTCOME_NAMES (char *[]){"draw", "winning", "losing", "?"}

typedef union {
    struct {
        U8 move;
        U8 outcome;
        U16 count;
    };
    U0 *ptr;
} Result;

#define NEW_THREAD_DEPTH_LIMIT 2

typedef struct {
    const Game *game;
    U16 depth;
    U16 max_depth;
    atomic_U16 *current_threads;
    atomic_bool done;
} RecursionArgs;

typedef struct {
    pthread_t thread;
    RecursionArgs *args;
    atomic_bool closed;
} Task;

U0 log_reasoning(const Game *game, Result *results, Result final) {
    FILE *file = fopen("peak_bot_reasoning.log", "w");
    if (file == NULL) return;
    for (U8 i = 0; i < game->amount_of_legal_moves; i++) {
        fprintf(
            file, "%u. [%s] %s in %u\n",
            i, game->legal_moves[i].notation,
            OUTCOME_NAMES[results[i].outcome],
            results[i].count
        );
    }
    U8 f = final.move;
    fprintf(
        file, "\n--> %u. [%s] %s in %u\n",
        f, game->legal_moves[f].notation,
        OUTCOME_NAMES[results[f].outcome],
        results[f].count
    );
    fclose(file);
}

U0 *peak_bot_recursion(U0 *pargs) {
    RecursionArgs *args = pargs;
    const Game *game = args->game;
    U16 depth = args->depth;
    U16 max_depth = args->max_depth;

    if (game->amount_of_legal_moves <= 0 && game->check) {
        args->done = true;
        return (Result){.count = depth, .outcome = LOSING}.ptr;
    } else if (game->draw) {
        args->done = true;
        return (Result){.count = depth, .outcome = DRAW}.ptr;
    } else if (max_depth != 0 && depth >= max_depth) {
        args->done = true;
        return (Result){.count = depth, .outcome = UNKNOWN}.ptr;
    }
    
    Result *results = malloc(game->amount_of_legal_moves * sizeof(Result));
    if (results == NULL) out_of_mem();
    Task *tasks = malloc(game->amount_of_legal_moves * sizeof(Task));
    if (tasks == NULL) out_of_mem();
    for (U8 i = 0; i < game->amount_of_legal_moves; i++) {
        Game test_game = copy_game(game);

        if (!do_move(&test_game, test_game.legal_moves[i].notation)) exit(1);
        //if (visualize) tui(&test_game, false);
        
        Game *test_game_heap = malloc(sizeof(Game));
        if (test_game_heap == NULL) out_of_mem();
        *test_game_heap = test_game;
        RecursionArgs *nargs = malloc(sizeof(RecursionArgs));
        if (nargs == NULL) out_of_mem();
        *nargs = (RecursionArgs){
            .game = test_game_heap,
            .depth = depth + 1,
            .max_depth = max_depth,
            .current_threads = args->current_threads
        };

        U16 current_threads = atomic_fetch_add(nargs->current_threads, 1);
        bool should_run_on_new_thread = (
            current_threads < threads &&
            (depth == 0 || depth < ((I32)max_depth - NEW_THREAD_DEPTH_LIMIT))
        );
        
        if (should_run_on_new_thread) {
            Task *task = &tasks[i];
            *task = (Task){.args = nargs};
            I32 r = pthread_create(&task->thread, NULL, peak_bot_recursion, nargs);
            if (r != 0) should_run_on_new_thread = false;
        }
        if (!should_run_on_new_thread) {
            (*nargs->current_threads)--;
            Result *result = &results[i];
            result->ptr = peak_bot_recursion(nargs);
            if (result->outcome == WINNING) {
                result->outcome = LOSING;
            } else if (result->outcome == LOSING) {
                result->outcome = WINNING;
            }
            result->move = i;
            close_game(&test_game);
            free(test_game_heap);
            free(nargs);
            tasks[i] = (Task){.closed = true};
        }
    }
    bool done = false;
    while (!done) {
        done = true;
        for (U8 i = 0; i < game->amount_of_legal_moves; i++) {
            Task *task = &tasks[i];
            if (!task->closed) {
                done = false;
                if (task->args->done) {
                    Result *result = &results[i];
                    pthread_join(task->thread, (U0 **)&result->ptr);
                    if (result->outcome == WINNING) {
                        result->outcome = LOSING;
                    } else if (result->outcome == LOSING) {
                        result->outcome = WINNING;
                    }
                    result->move = i;
                    (*args->current_threads)--;
                    close_game((Game *)task->args->game);
                    free((Game *)task->args->game);
                    free(task->args);
                    task->closed = true;
                }
            }
        }
    }
    U8 default_result = jonkler(game, NULL);
    Result final = results[default_result];
    for (U8 i = 0; i < game->amount_of_legal_moves; i++) {
        if (i != default_result) {
            Result this = results[i];
            bool this_wins = this.outcome == WINNING;
            bool this_draws = this.outcome == DRAW || this.outcome == UNKNOWN;
            bool this_loses = this.outcome == LOSING;
            bool winning = final.outcome == WINNING;
            bool drawing = final.outcome == DRAW || final.outcome == UNKNOWN;
            bool losing = final.outcome == LOSING;
            if (
                (this_wins && (!winning || this.count < final.count)) ||
                (this_draws && (losing || (!winning && this.count > final.count))) ||
                (this_loses && losing && this.count > final.count)
            ) final = this;
        }
    }
    if (depth == 0) log_reasoning(game, results, final);
    free(tasks);
    free(results);
    args->done = true;
    return final.ptr;
}

U8 peak_bot(const Game *game, U0 *max_depth) {
    Result result;
    atomic_U16 current_threads = 0;
    result.ptr = peak_bot_recursion(&(RecursionArgs){
        .game = game, .max_depth = (uintptr_t)max_depth,
        .current_threads = &current_threads
    });
    return result.move;
}

