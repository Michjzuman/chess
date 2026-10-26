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
#define OUTCOME_NAMES (char *[]){"?", "draw", "winning", "losing"}

typedef struct {
    U8 move;
    U8 outcome: 4;
    U16 count;
} Result;

typedef struct {
    U8 outcome;
    U16 result_count;
    U8 branch_count;
    U0 *branches;
} MemoryNode;
#define MEMORY_NODE_OUTCOME_BIAS 250

typedef _Atomic(MemoryNode) atomic_MemoryNode;

#define MRPOOP_DEPTH 4

atomic_MemoryNode research_memory;

#define NEW_THREAD_DEPTH_LIMIT 2

typedef struct {
    const Game *game;
    U16 depth;
    U16 max_depth;
    atomic_U16 *current_threads;
    atomic_bool done;
    bool is_on_new_thread: 1;
    bool is_mrpoop: 1;
    atomic_MemoryNode *memory_node;
    Result result;
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

U0 close_memory(atomic_MemoryNode *node) {
    MemoryNode n = atomic_load(node);
    if (n.branches == NULL) return;
    for (U8 i = 0; i < n.branch_count; i++) {
        close_memory(&((atomic_MemoryNode *)n.branches)[i]);
    }
    free(n.branches);
}

U0 *peak_bot_recursion(U0 *pargs) {
    RecursionArgs *args = pargs;
    const Game *game = args->game;
    U16 depth = args->depth;
    U16 max_depth = args->max_depth;

    MemoryNode n = atomic_load(args->memory_node);

    if (args->is_mrpoop && n.outcome != UNKNOWN) {
        args->done = true;
        if (args->is_on_new_thread) atomic_fetch_sub(args->current_threads, 1);
        args->result = (Result){
            .count = n.result_count,
            .outcome = n.outcome
        };
        return NULL;
    }

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
        n.outcome = outcome;
        atomic_store(args->memory_node, n);
        if (args->is_on_new_thread) atomic_fetch_sub(args->current_threads, 1);
        args->result = (Result){.count = depth, .outcome = outcome};
        return NULL;
    }
    
    Result *results = malloc(game->amount_of_legal_moves * sizeof(Result));
    if (results == NULL) out_of_mem();
    Task *tasks = malloc(game->amount_of_legal_moves * sizeof(Task));
    if (tasks == NULL) out_of_mem();

    if (n.branches == NULL) {
        n.branch_count = game->amount_of_legal_moves;
        n.branches = calloc(n.branch_count, sizeof(atomic_MemoryNode));
        atomic_store(args->memory_node, n);
    }
    for (U8 i = 0; i < game->amount_of_legal_moves; i++) {
        Game test_game = copy_game(game);

        if (do_move(&test_game, test_game.legal_moves[i].notation) == MOVE_WAS_ILLEGAL) exit(1);
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
            .is_mrpoop = args->is_mrpoop,
            .memory_node = &((atomic_MemoryNode *)n.branches)[i]
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
            peak_bot_recursion(nargs);
            *result = nargs->result;
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
                    pthread_join(task->thread, NULL);
                    *result = task->args->result;
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
            bool losing = final.outcome == LOSING;
            if (
                (this_wins && (!winning || this.count < final.count)) ||
                (this_draws && (losing || (!winning && this.count > final.count))) ||
                (this_loses && losing && this.count > final.count)
            ) final = this;
        }
    }
    n.outcome = final.outcome;
    n.result_count = final.count + 1;
    atomic_store(args->memory_node, n);
    if (depth == 0) log_reasoning(game, results, final);
    free(tasks);
    free(results);
    args->result = final;
    args->done = true;
    if (args->is_on_new_thread) atomic_fetch_sub(args->current_threads, 1);
    return NULL;
}

atomic_bool write_memory;

#define MEMORY_DIRPATH "./research_memory"
#define MEMORY_TMP_DIRPATH "./research_memory/.tmp"
#define MEMORY_FILE_SIZE_LIMIT (U64)100000000

U0 write_memory_file(U8 *content, U64 *i, U64 *fi) {
    char path[1024];
    snprintf(path, 1024, "%s/%llu.mem", MEMORY_TMP_DIRPATH, *fi);
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        fprintf(stderr, "failed to write %s\n", path);
        return;
    }
    fwrite(content, sizeof(U8), *i, file);
    fclose(file);
    (*i) = 0;
    (*fi)++;
}

U0 write_byte_to_file(U8 *content, U64 *i, U64 *fi, U8 byte) {
    if (*i > 0 && *i % MEMORY_FILE_SIZE_LIMIT == 0) {
        write_memory_file(content, i, fi);
    }
    content[*i] = byte; (*i)++;
}

U0 memory_writer_recursion(atomic_MemoryNode *node, U8 *content, U64 *i, U64 *fi) {
    MemoryNode n = atomic_load(node);
    write_byte_to_file(content, i, fi, n.outcome);
    write_byte_to_file(content, i, fi, n.result_count>>8);
    write_byte_to_file(content, i, fi, n.result_count);
    write_byte_to_file(content, i, fi, n.branch_count);
    for (U8 i2 = 0; i2 < n.branch_count; i2++) {
        memory_writer_recursion(&((atomic_MemoryNode *)n.branches)[i2], content, i, fi);
    }
}

U0 *memory_writer(U0 *args) {
    {
        struct stat st1;
        if (stat(MEMORY_DIRPATH, &st1) != 0 || !S_ISDIR(st1.st_mode)) {
            mkdir(MEMORY_DIRPATH, 0700);
        }
        struct stat st2;
        if (stat(MEMORY_TMP_DIRPATH, &st2) != 0 || !S_ISDIR(st2.st_mode)) {
            mkdir(MEMORY_TMP_DIRPATH, 0700);
        }
    }
    while (write_memory) {
        U8 *content = malloc(MEMORY_FILE_SIZE_LIMIT);
        U64 i = 0, fi = 0;
        memory_writer_recursion(&research_memory, content, &i, &fi);
        write_memory_file(content, &i, &fi);
        free(content);
        char cmd[1024];
        snprintf(cmd, 1024, "mv %s/* %s", MEMORY_TMP_DIRPATH, MEMORY_DIRPATH);
        system(cmd);
    }
    return NULL;
}

U0 read_memory_file(U8 *content, U64 *i, U64 *fi) {
    char path[1024];
    snprintf(path, 1024, "%s/%llu.mem", MEMORY_DIRPATH, *fi);
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        fprintf(stderr, "failed to read %s\n", path);
        return;
    }
    fread(content, sizeof(U8), *i, file);
    fclose(file);
    (*i) = 0;
    (*fi)++;
}

U8 read_byte_from_file(U8 *content, U64 *i, U64 *fi) {
    if (*i > 0 && *i % MEMORY_FILE_SIZE_LIMIT == 0) {
        read_memory_file(content, i, fi);
    }
    U8 result = content[*i]; (*i)++;
    return result;
}

U0 memory_reader_recursion(atomic_MemoryNode *node, U8 *content, U64 *i, U64 *fi) {
    MemoryNode n;
    n.outcome = read_byte_from_file(content, i, fi);
    n.result_count = (
        (read_byte_from_file(content, i, fi)<<8) |
        read_byte_from_file(content, i, fi)
    );
    n.branch_count = read_byte_from_file(content, i, fi);
    n.branches = n.branch_count ? calloc(n.branch_count, sizeof(atomic_MemoryNode)) : NULL;
    atomic_store(node, n);
    for (U8 i2 = 0; i2 < n.branch_count; i2++) {
        memory_reader_recursion(
            &((atomic_MemoryNode *)n.branches)[i2],
            content, i, fi
        );
    }
}

U0 memory_reader(U0) {
    {
        struct stat st1;
        if (stat(MEMORY_DIRPATH, &st1) != 0 || !S_ISDIR(st1.st_mode)) return;
        struct stat st2;
        char path[1024];
        snprintf(path, 1024, "%s/0.mem", MEMORY_DIRPATH);
        if (stat(path, &st2) != 0) return;
    }
    MemoryNode n = atomic_load(&research_memory);
    if (n.branches == NULL && n.branch_count == 0) {
        U8 *content = malloc(MEMORY_FILE_SIZE_LIMIT);
        U64 i = MEMORY_FILE_SIZE_LIMIT, fi = 0;
        read_memory_file(content, &i, &fi);
        memory_reader_recursion(&research_memory, content, &i, &fi);
        free(content);
    }
}

#ifdef RAYLIB
    #define WIN_W 1400
    #define WIN_H 800

    typedef struct {
        U64 *w;
        U16 h;
    } Visualization;

    Visualization vis;

    U0 visualization_recursion(atomic_MemoryNode *node, U16 depth) {
        if (depth + 1 > vis.h) {
            vis.h++;
            vis.w = realloc(vis.w, vis.h * sizeof(U64));
        }
    }

    U0 visualize(U0) {
        typedef struct {
            atomic_MemoryNode *node;
            float x, y, span;
            float parent_x, parent_y;
            U16 depth;
        } DrawItem;

        InitWindow(WIN_W, WIN_H, "chess research memory");
        SetTargetFPS(30);
        while (!WindowShouldClose()) {
            memory_reader();
            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("Research memory tree (depth limited)", 20, 18, 20, DARKGRAY);

            U64 capacity = 256, top = 0, drawn = 0;
            DrawItem *stack = malloc(capacity * sizeof(DrawItem));
            if (stack != NULL) {
                stack[top++] = (DrawItem){
                    .node = &research_memory,
                    .x = WIN_W / 2.0f,
                    .y = 70.0f,
                    .span = WIN_W - 40.0f
                };
                while (top > 0 && drawn < 5000) {
                    DrawItem item = stack[--top];
                    MemoryNode n = atomic_load(item.node);
                    if (item.depth > 0) {
                        DrawLine(item.parent_x, item.parent_y, item.x, item.y, LIGHTGRAY);
                    }
                    Color color = n.outcome == WINNING ? GREEN :
                        n.outcome == LOSING ? RED :
                        n.outcome == DRAW ? BLUE : GRAY;
                    DrawCircle((int)item.x, (int)item.y, 9, color);
                    char label[48];
                    snprintf(label, sizeof(label), "%u / %u", n.outcome, n.result_count);
                    DrawText(label, (int)item.x + 11, (int)item.y - 7, 12, DARKGRAY);
                    drawn++;

                    if (item.depth >= 8 || n.branches == NULL || n.branch_count == 0) continue;
                    float child_y = item.y + 76.0f;
                    float child_span = item.span / n.branch_count;
                    for (U8 i = 0; i < n.branch_count; i++) {
                        if (top == capacity) {
                            capacity *= 2;
                            DrawItem *grown = realloc(stack, capacity * sizeof(DrawItem));
                            if (grown == NULL) break;
                            stack = grown;
                        }
                        float child_x = item.x - item.span / 2.0f + child_span * (i + 0.5f);
                        stack[top++] = (DrawItem){
                            .node = &((atomic_MemoryNode *)n.branches)[i],
                            .x = child_x,
                            .y = child_y,
                            .span = child_span,
                            .parent_x = item.x,
                            .parent_y = item.y,
                            .depth = item.depth + 1
                        };
                    }
                }
                free(stack);
            }
            DrawText("? unknown   blue draw   green winning   red losing", 20, WIN_H - 30, 16, DARKGRAY);
            EndDrawing();
        }
        CloseWindow();
        exit(0);
    }

    int main() {
        visualize();
    }
#endif

U8 get_bot_move(const Game *game, RecursionArgs *args) {
    memory_reader();
    write_memory = true;
    pthread_t memory_writer_thread;
    pthread_create(&memory_writer_thread, NULL, memory_writer, NULL);
    Result result;
    atomic_U16 current_threads = 0;
    args->current_threads = &current_threads;
    {
        Game test_game = new_game();
        atomic_MemoryNode *node = &research_memory;
        for (U16 i = 0; i < game->amount_of_moves; i++) {
            MemoryNode n = atomic_load(node);
            if (n.branches == NULL) {
                n.branch_count = test_game.amount_of_legal_moves;
                n.branches = calloc(n.branch_count, sizeof(atomic_MemoryNode));
                atomic_store(node, n);
            }
            U8 move = do_move(&test_game, game->moves[i]);
            node = &((atomic_MemoryNode *)n.branches)[move];
        }
        args->memory_node = node; // why yo bulleh meh
    }
    peak_bot_recursion(args);
    result = args->result;
    write_memory = false;
    pthread_join(memory_writer_thread, NULL);
    return result.move;
}

U8 peak_bot(const Game *game, U0 *max_depth) {
    return get_bot_move(game, &(RecursionArgs){
        .game = game, .max_depth = (uintptr_t)max_depth
    });
}

U8 mrpoop(const Game *game, U0 *args) {
    return get_bot_move(game, &(RecursionArgs){
        .game = game, .max_depth = MRPOOP_DEPTH, .is_mrpoop = true
    });
}
