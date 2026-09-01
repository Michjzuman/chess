#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <dirent.h>

#include "chess.h"
#include "bots.h"
#include "nn.h"

#define max_name_len 9
#define max_path_len 21
#define survivors 7
#define AMOUNT_OF_PLAYERS 32

const char *dirpath = "./brains";

typedef atomic_int_least8_t atomic_U8;

enum {NN_TPlayer, Bot_TPlayer};

typedef union {
    NN *nn;
    PF bot;
} TypeTPlayerArg;

typedef struct {
    char name[max_name_len];
    U32 points;
    U8 type;
    TypeTPlayerArg arg;
    atomic_U8 in_use;
    U32 id;
    bool senior;
} TPlayer;

typedef struct {
    U8 amount_of_threads;
    U8 amount_of_players;
    U8 target_amount_of_players;
    TPlayer *players;
    U32 round_count;
} Tournament;

typedef struct {
    pthread_t thread;
    U8 p[2];
    Tournament *t;
    atomic_bool done;
} RunningGame;

struct Bot {
    char name[max_name_len];
    PF function;
};

static const struct Bot bots[] = {/*
    {"jonkler", jonkler},
    {"thief", thief},
    {"murderer", murderer}
*/};
const U8 amount_of_bots = sizeof(bots) / sizeof(struct Bot);

typedef struct {
    U32 id[2];
    U8 result;
} MemorizedGame;

typedef struct {
    U32 amount_of_memorized_games;
    MemorizedGame *memorized_games;
} GameMemory;

GameMemory game_memory;

static U32 get_id() {
    static U32 next_id = 0;
    return next_id++;
}

static char *get_path(char *name) {
    char *path = malloc(max_path_len * sizeof(char));
    if (path == NULL) out_of_mem();
    snprintf(
        path, max_path_len,
        "%s/%s.nn", dirpath, name
    );
    return path;
}

static U0 title(char *text) {
    printf("\033[46m\033[1m[ %s ]\033[0m\n", text);
}

static U0 rand_name(char *name) {
    static const char abc[] = (
        "abcdefghijklmnopqrstuvwxyz_"
    );
    U8 len = rand() % (max_name_len - 1) + 1;
    for (U8 i2 = 0; i2 < len; i2++) {
        name[i2] = abc[(U8)rand() % strlen(abc)];
    }
    name[len]= '\0';
}

static U32 open_tplayers(Tournament *t) {
    t->amount_of_players = t->target_amount_of_players;

    DIR *dir = opendir(dirpath);
    if (dir == NULL) {
        fprintf(stderr, "path %s could not be opened\n", dirpath);
        exit(1);
    }

    struct dirent *entry;

    U32 count = 0;

    while ((entry = readdir(dir)) != NULL) {
        char *name = entry->d_name;
        if (
            strcmp(name, ".") == 0 ||
            strcmp(name, "..") == 0 ||
            strcmp(name + strlen(name) - 3, ".nn") != 0
        ) continue;
        name[strlen(name) - 3] = '\0';
        char *path = get_path(name);
        printf("- \033[32m%s\033[0m found in %s\n", name, path);
        TPlayer player = {
            .type = NN_TPlayer, .id = get_id(), .senior = false
        };
        strcpy(player.name, name);
        if (t->amount_of_players <= count + amount_of_bots) {
            t->amount_of_players++;
            t->players = realloc(t->players, t->amount_of_players * sizeof(TPlayer));
        }
        if (t->players == NULL) out_of_mem();
        t->players[count + amount_of_bots] = player;
        free(path);
        count++;
    }

    closedir(dir);
    return count;
}

U0 *birth(U0 *name) {
    NN *nn = malloc(sizeof(NN));
    if (nn == NULL) out_of_mem();
    *nn = new_chess_nn();
    char *path = get_path(name);
    save_nn(nn, path);
    close_nn(nn);
    free(path);
    free(name);
    return NULL;
}

static U0 give_birth(Tournament *t, U8 start) {
    t->amount_of_players = t->target_amount_of_players;
    U8 used_threads = 0;
    pthread_t threads[t->amount_of_threads];
    U8 players_threads_i[t->amount_of_threads];
    t->players = realloc(t->players, t->target_amount_of_players * sizeof(TPlayer));
    if (t->players == NULL) out_of_mem();
    for (U8 i = start; i < t->target_amount_of_players; i++) {
        bool found = false;
        while (!found) {
            rand_name(t->players[i].name);
            found = true;
            for (U8 i2 = 0; i2 < i; i2++) {
                if (strcmp(t->players[i].name, t->players[i2].name) == 0) {
                    found = false;
                }
            }
        }
        char *heap_name = malloc(
            max_name_len * sizeof(char)
        );
        if (heap_name == NULL) out_of_mem();
        strcpy(heap_name, t->players[i].name);
        t->players[i].type = NN_TPlayer;
        t->players[i].id = get_id();
        t->players[i].senior = false;
        pthread_create(&threads[used_threads], NULL, birth, heap_name);
        players_threads_i[used_threads] = i;
        used_threads++;
        if (
            used_threads >= t->amount_of_threads ||
            i == t->amount_of_players - 1
        ) {
            for (U8 i1 = 0; i1 < used_threads; i1++) {
                pthread_join(threads[i1], NULL);
                printf(
                    "%d/%d: \033[32m%s\033[0m was \033[36mborn\033[0m!\n",
                    players_threads_i[i1] + 1 - start,
                    t->amount_of_players - start,
                    t->players[players_threads_i[i1]].name
                );
            }
            used_threads = 0;
        }
    }
}

static U0 initial_birth(Tournament *t) {
    title("preparation");
    t->players = malloc(t->target_amount_of_players * sizeof(TPlayer));
    if (t->players == NULL) out_of_mem();
    for (U8 i = 0; i < amount_of_bots; i++) {
        t->players[i] = (TPlayer){
            .arg.bot =  bots[i].function,
            .type = Bot_TPlayer, .id = get_id(), .senior = false
        };
        strcpy(t->players[i].name, bots[i].name);
    }
    const U32 existing_players = amount_of_bots + open_tplayers(t);
    give_birth(t, existing_players);
    game_memory.amount_of_memorized_games = 0;
    game_memory.memorized_games = malloc(0);
}

U0 *simulate_game(U0 *simargs) {
    RunningGame *game = simargs;
    uintptr_t winner;

    bool memorized = false;
    if (
        game->t->players[game->p[0]].senior &&
        game->t->players[game->p[0]].senior
    ) {
        for (U32 i = 0; i < game_memory.amount_of_memorized_games; i++) {
            MemorizedGame *memorized_game = &game_memory.memorized_games[i];
            if (
                game->t->players[game->p[0]].id == memorized_game->id[0] &&
                game->t->players[game->p[1]].id == memorized_game->id[1]
            ) {
                memorized = true;
                winner = memorized_game->result;
            }
        }
    }
    if (!memorized) {
        //printf("----- [start] ---------------------------\n");
        struct {PF function; U0 *args;} pfs[2];
    
        for (U8 i = 0; i < 2; i++) {
            TPlayer *player = &game->t->players[game->p[i]];
            if (player->type == NN_TPlayer) {
                pfs[i].function = neural_network;
                pfs[i].args = player->arg.nn;
            } else {
                pfs[i].function = player->arg.bot;
                pfs[i].args = NULL;
            }
        }
    
        winner = play(bg,
            pfs[0].function, pfs[0].args,
            pfs[1].function, pfs[1].args
        );
        //printf("----- [end] ---------------------------\n");
    }

    for (U8 i = 0; i < 2; i++) {
        TPlayer *player = &game->t->players[game->p[i]];
        atomic_U8 *in_use = &player->in_use;
        if (player->type == NN_TPlayer) {
            atomic_store(in_use, atomic_load(in_use) - 1);
            if (atomic_load(in_use) <= 0) {
                /*printf(
                    "#################################### %s --> free\n",
                    player->name
                );*/
                close_nn(player->arg.nn);
            }
        }
    }

    atomic_store(&game->done, true);

    return (U0 *)winner;
}

static U0 start_simulation(RunningGame *game) {
    for (U32 p = 0; p < 2; p++) {
        TPlayer *player = &game->t->players[game->p[p]];
        if (player->type == NN_TPlayer) {
            atomic_store(&player->in_use, atomic_load(&player->in_use) + 1);
        }
        if (atomic_load(&player->in_use) == 1) {
            char *path = get_path(player->name);
            /*printf(
                "#################################### %s <-- %s\n",
                player->name, path
            );*/
            player->arg.nn = open_nn(path);
            free(path);
        }
    }
    pthread_create(
        &game->thread, NULL, simulate_game, game
    );
}

static U0 end_simulation(
    Tournament *t, RunningGame *all_games, U32 total_games,
    U32 total_game_count, U32 next_game, GameMemory *next_game_memory
) {
    bool found = false;
    while (!found) {
        for (U32 i2 = 0; i2 < next_game; i2++) {
            if (atomic_load(&all_games[i2].done)) {
                uintptr_t winner;
                pthread_join(all_games[i2].thread, (U0 *)&winner);

                TPlayer *players[2];
                players[0] = &t->players[all_games[i2].p[0]];
                players[1] = &t->players[all_games[i2].p[1]];

                MemorizedGame *memorized_game = &next_game_memory->memorized_games[total_game_count];
                memorized_game->result = winner;
                memorized_game->id[0] = players[0]->id;
                memorized_game->id[1] = players[1]->id;
                
                total_game_count++;
                printf(
                    "%d/%d: \033[%sm%s\033[0m vs. \033[%sm%s\033[0m -> ",
                    total_game_count, total_games,
                    players[0]->type == Bot_TPlayer ? "92" : "32",
                    players[0]->name,
                    players[1]->type == Bot_TPlayer ? "92" : "32",
                    players[1]->name
                );
                if (winner == 0) {
                    printf("\033[90mdraw\033[0m");
                    players[0]->points++;
                    players[1]->points++;
                } else {
                    printf(
                        "\033[33mwinner\033[0m: \033[%sm%s\033[0m",
                        t->players[all_games[i2].p[winner - 1]].type == Bot_TPlayer ? "92" : "32",
                        t->players[all_games[i2].p[winner - 1]].name
                    );
                    t->players[all_games[i2].p[winner - 1]].points += 2;
                }
                printf("\n");
                
                atomic_store(&all_games[i2].done, false);
                found = true;
                break;
            }
        }
    }
}

static U0 play_round(Tournament *t) {
    t->round_count++;
    {
        char title_text[20];
        snprintf(title_text, 20, "round %u:", t->round_count);
        title(title_text);
    }
    U32 total_games = 0;
    for (U8 i = 0; i < t->amount_of_players; i++) {
        total_games += (t->amount_of_players - i - 1) * 2;
    }
    U32 total_game_count = 0;

    for (U8 i = 0; i < t->amount_of_players; i++) {
        t->players[i].points = 0;
        atomic_store(&t->players[i].in_use, 0);
    }
    
    GameMemory next_game_memory;
    next_game_memory.amount_of_memorized_games = total_games;
    next_game_memory.memorized_games = malloc(total_games * sizeof(MemorizedGame));
    if (next_game_memory.memorized_games == NULL) out_of_mem();
    RunningGame all_games[total_games];

    {
        U16 i = 0;
        for (U8 i1 = 0; i1 < t->amount_of_players; i1++) {
            for (U8 i2 = i1 + 1; i2 < t->amount_of_players; i2++) {
                for (U8 color = 0; color < 2; color++) {
                    RunningGame game; game.t = t;
                    game.p[0] = color == 0 ? i1 : i2;
                    game.p[1] = color == 0 ? i2 : i1;
                    atomic_store(&game.done, false);
                    all_games[i] = game;
                    i++;
                }
            }
        }
    }

    for (U32 i = 0; i < t->amount_of_threads && i < total_games; i++) {
        start_simulation(&all_games[i]);
    }
    U32 next_game = t->amount_of_threads; 
    while (next_game < total_games) {
        end_simulation(t, all_games, total_games, total_game_count, next_game, &next_game_memory);
        start_simulation(&all_games[next_game]);
        next_game++;
        total_game_count++;
    }
    for (U8 i = 0; i < t->amount_of_threads && i < total_games; i++) {
        end_simulation(t, all_games, total_games, total_game_count, total_games, &next_game_memory);
        total_game_count++;
    }

    free(game_memory.memorized_games);
    game_memory = next_game_memory;
}

static int compare_tplayers(const U0 *a, const U0 *b) {
    U32 ap = ((TPlayer *)a)->points;
    U32 bp = ((TPlayer *)b)->points;
    return (ap < bp) - (ap > bp);
}

static U0 sort_players(Tournament *t) {
    title("ranking");
    qsort(
        t->players, t->amount_of_players,
        sizeof(TPlayer), compare_tplayers
    );
    for (U8 i = 0; i < survivors && i < t->amount_of_players; i++) {
        t->players[i].senior = true;
        printf(
            "%d. \033[32m%s\033[0m (%d)\n", i + 1,
            t->players[i].name, t->players[i].points
        );
    }
}

static U0 elimination(Tournament *t) {
    title("elimination");
    for (U8 i = survivors; i < t->amount_of_players; i++) {
        if (t->players[i].type == NN_TPlayer) {
            char *path = get_path(t->players[i].name);
            if (remove(path) == 0) {
                printf(
                    "%d/%d \033[32m%s\033[0m has \033[31mdied\033[0m\n",
                    i - survivors + 1, t->amount_of_players - survivors, t->players[i].name
                );
            } else {
                printf(
                    "%d/%d \033[32m%s\033[0m was \033[31meliminated\033[0m but his file could not be removed\n",
                    i - survivors + 1, t->amount_of_players - survivors, t->players[i].name
                );
            }
            free(path);
        } else {
            printf(
                "%d/%d \033[32m%s\033[0m was \033[31meliminated\033[0m\n",
                i - survivors + 1, t->amount_of_players - survivors, t->players[i].name
            );
        }
    }
}

static U0 repopulation(Tournament *t) {
    title("repopulation");
    U32 current_amount_of_players = survivors;
    for (U8 i = 0; i < survivors; i++) {
        if (t->players[i].type == NN_TPlayer) {
            char *parent_nn_path = get_path(t->players[i].name);
            NN *parent_nn = open_nn(parent_nn_path);
            for (U8 i2 = 0; i2 < i; i2++) {
                TPlayer child;
                child.type = NN_TPlayer;
                child.id = get_id();
                child.senior = false;
                bool found = false;
                while (!found) {
                    rand_name(child.name);
                    found = true;
                    for (U8 i3 = 0; i3 < current_amount_of_players; i3++) {
                        if (strcmp(child.name, t->players[i3].name) == 0) {
                            found = false;
                        }
                    }
                }
                char *child_nn_path = get_path(child.name);
                NN *child_nn = mutate_nn(parent_nn);
                save_nn(child_nn, child_nn_path);
                t->players[current_amount_of_players] = child;
                free(child_nn_path);
                close_nn(child_nn);
                printf(
                    "%d/%d: \033[32m%s\033[0m SON of \033[32m%s\033[0m was \033[36mborn\033[0m!\n",
                    i2 + 1, i,
                    t->players[current_amount_of_players].name,
                    t->players[i].name
                );
                current_amount_of_players++;
            }
            free(parent_nn_path);
            close_nn(parent_nn);
        }
    }
    give_birth(t, current_amount_of_players);
    t->amount_of_players = t->target_amount_of_players;
}

U0 tournament(U8 amount_of_threads) {
    srand(time(NULL));
    
    Tournament t;
    t.amount_of_threads = amount_of_threads;
    t.target_amount_of_players = AMOUNT_OF_PLAYERS;
    t.round_count = 0;

    initial_birth(&t);
    
    while (true) {
        play_round(&t);
        sort_players(&t);
        elimination(&t);
        repopulation(&t);
    }
    
    title("end");
    free(t.players);
    free(game_memory.memorized_games);
}

