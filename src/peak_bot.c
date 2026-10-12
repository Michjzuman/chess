#include "chess.h"
#include "tui.h"
#include "bots.h"

#include <pthread.h>
#include <stdatomic.h>
#include <sys/stat.h>
#ifdef RAYLIB
    #include <raylib.h>
#endif

typedef atomic_int_least16_t atomic_U16;
typedef atomic_int_least64_t atomic_U64;

enum {UNKNOWN, DRAW, WINNING, LOSING, OUTCOME_MAX};
#define OUTCOME_NAMES (char *[]){"draw", "winning", "losing", "?"}

typedef union {
    struct {
        U8 move;
        U8 outcome: 4;
        U16 count;
    };
    U0 *ptr;
} Result;

typedef struct {
    union {
        U8 outcome;
        U8 branch_count;
    };
    U0 *branches;
} MemoryNode;

typedef _Atomic(MemoryNode) atomic_MemoryNode;

#define MEMBOT_DEPTH 6

atomic_MemoryNode research_memory;

#define NEW_THREAD_DEPTH_LIMIT 2

typedef struct {
    const Game *game;
    U16 depth;
    U16 max_depth;
    atomic_U16 *current_threads;
    atomic_bool done;
    bool is_on_new_thread: 1;
    bool is_membot: 1;
    atomic_MemoryNode *memory_node;
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

U0 memorize_outcome(atomic_MemoryNode *node, U8 outcome) {
    atomic_store(node, (MemoryNode){.outcome = outcome});
}

U0 fork_memory(atomic_MemoryNode *node, U8 count) {
    atomic_store(
        node, ((MemoryNode){
            .branch_count = count,
            .branches = calloc(count, sizeof(atomic_MemoryNode))
        })
    );
}

U0 close_memory(atomic_MemoryNode *node) {
    MemoryNode n = atomic_load(node);
    if (n.branches == NULL) return;
    for (U8 i = 0; i < n.branch_count; i++) {
        close_memory(&n.branches[i]);
    }
    free(n.branches);
}

U0 *peak_bot_recursion(U0 *pargs) {
    RecursionArgs *args = pargs;
    const Game *game = args->game;
    U16 depth = args->depth;
    U16 max_depth = args->max_depth;

    U8 outcome = OUTCOME_MAX;
    if (game->amount_of_legal_moves <= 0 && game->check) {
        outcome = LOSING;
    } else if (game->draw) {
        outcome = DRAW;
    } else if (max_depth != 0 && depth >= max_depth) {
        outcome = UNKNOWN;
    }
    if (outcome < OUTCOME_MAX) {
        args->done = true;
        memorize_outcome(args->memory_node, outcome);
        return (Result){.count = depth, .outcome = outcome}.ptr;
    }
    
    Result *results = malloc(game->amount_of_legal_moves * sizeof(Result));
    if (results == NULL) out_of_mem();
    Task *tasks = malloc(game->amount_of_legal_moves * sizeof(Task));
    if (tasks == NULL) out_of_mem();

    fork_memory(args->memory_node, game->amount_of_legal_moves);
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
            .current_threads = args->current_threads,
            .is_membot = args->is_membot,
            .memory_node = &((atomic_MemoryNode *)atomic_load(args->memory_node).branches)[i]
        };

        U16 current_threads = atomic_fetch_add(nargs->current_threads, 1);
        bool should_run_on_new_thread = (
            current_threads < (threads - 1) &&
            (depth == 0 || depth < ((I32)max_depth - NEW_THREAD_DEPTH_LIMIT))
        );
        
        if (should_run_on_new_thread) {
            Task *task = &tasks[i];
            *task = (Task){.args = nargs};
            nargs->is_on_new_thread = true;
            I32 r = pthread_create(&task->thread, NULL, peak_bot_recursion, nargs);
            if (r != 0) should_run_on_new_thread = false;
        }
        if (!should_run_on_new_thread) {
            nargs->is_on_new_thread = false;
            atomic_fetch_sub(args->current_threads, 1);
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
    if (args->is_on_new_thread) atomic_fetch_sub(args->current_threads, 1);
    return final.ptr;
}

atomic_bool write_memory;

#ifdef RAYLIB
    bool window_shown = true;

    U0 visualize(U0) {
        InitWindow(800, 450, "chess");
        SetTargetFPS(30);
        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);
            EndDrawing();
        }
        CloseWindow();
        exit(0);
    }
#endif

#define MEMORY_DIRPATH "./research_memory"
#define MEMORY_TMP_DIRPATH "./research_memory/.tmp"
#define MEMORY_FILE_SIZE_LIMIT (U64)100000000

U0 write_memory_file(U8 *content, U64 *i, U64 *fi) {
    char path[1024];
    snprintf(
        path, 1024, "%s/%llu.mem",
        MEMORY_TMP_DIRPATH, *fi
    );
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        fprintf(stderr, "failed to open %s\n", path);
        return;
    }
    fwrite(content, sizeof(U8), *i, file);
    fclose(file);
    (*fi)++;
}

U0 memory_writer_recursion(atomic_MemoryNode *node, U8 *content, U64 *i, U64 *fi) {
    MemoryNode n = atomic_load(node);
    if (*i > 0 && *i % MEMORY_FILE_SIZE_LIMIT == 0) {
        write_memory_file(content, i, fi);
        *i = 0;
    }
    if (n.branches == NULL) {
        content[*i] = n.outcome + 250; (*i)++;
    } else {
        content[*i] = n.branch_count; (*i)++;
        for (U8 i2 = 0; i2 < n.branch_count; i2++) {
            memory_writer_recursion(
                &((atomic_MemoryNode *)n.branches)[i2], content, i, fi
            );
        }
    }
}

U0 *memory_writer_agent(U0 *args) {
    struct stat st1;
    if (stat(MEMORY_DIRPATH, &st1) != 0 || !S_ISDIR(st1.st_mode)) {
        mkdir(MEMORY_DIRPATH, 0700);
    }
    struct stat st2;
    if (stat(MEMORY_TMP_DIRPATH, &st2) != 0 || !S_ISDIR(st2.st_mode)) {
        mkdir(MEMORY_TMP_DIRPATH, 0700);
    }
    while (write_memory) {
        U8 *content = malloc(MEMORY_FILE_SIZE_LIMIT);
        U64 i = 0, fi = 0;
        memory_writer_recursion(&research_memory, content, &i, &fi);
        write_memory_file(content, &i, &fi);
        free(content);
        char cmd[1024];
        snprintf(cmd, 1024, "cp %s/* %s", MEMORY_TMP_DIRPATH, MEMORY_DIRPATH);
        system(cmd);
    }
    return NULL;
}

U8 get_bot_move(const Game *game, RecursionArgs *args) {
    write_memory = true;
    pthread_t memory_writer;
    pthread_create(&memory_writer, NULL, memory_writer_agent, NULL);
    #ifdef RAYLIB
        if (!window_shown) {
            pid_t pid = fork();
            if (pid == 0) {
                getpid();
                window_shown = true;
                visualize();
            }
        }
    #endif
    Result result;
    atomic_U16 current_threads = 0;
    args->current_threads = &current_threads;
    args->memory_node = &research_memory;
    result.ptr = peak_bot_recursion(args);
    write_memory = false;
    pthread_join(memory_writer, NULL);
    return result.move;
}

U8 peak_bot(const Game *game, U0 *max_depth) {
    return get_bot_move(game, &(RecursionArgs){
        .game = game, .max_depth = (uintptr_t)max_depth
    });
}

U8 membot(const Game *game, U0 *args) {
    return get_bot_move(game, &(RecursionArgs){
        .game = game, .max_depth = MEMBOT_DEPTH, .is_membot = true
    });
}
