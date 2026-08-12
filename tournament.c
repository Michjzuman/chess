#include <pthread.h>
#include <stdatomic.h>

#include "chess.h"
#include "bots.h"
#include "nn.h"
#include "tui.h"

#define max_name_len 9
#define max_path_len 21

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
} TPlayer;

typedef struct {
    U8 amount_of_threads;
    U8 amount_of_players;
    TPlayer *players;
    U32 round_count;
} Tournament;

typedef struct {
    pthread_t thread;
    U8 p[2];
    Tournament *t;
    atomic_bool done;
} RunningGame;

static char *get_path(char *name) {
    char *path = malloc(max_path_len * sizeof(char));
    if (path == NULL) out_of_mem();
    snprintf(
        path, max_path_len,
        "./brains/%s.nn", name
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

U0 *birth(U0 *name) {
    NN *nn = malloc(sizeof(NN));
    if (nn == NULL) out_of_mem();
    *nn = new_chess_nn();
    char *path = get_path(name);
    save_nn(nn, path);
    close_nn(nn);
    free(nn);
    free(path);
    free(name);
    return name;
}

static U0 initial_birth(Tournament *t) {
    title("preparation");
    t->round_count = 0;
    t->players = malloc(t->amount_of_players * sizeof(TPlayer));
    if (t->players == NULL) out_of_mem();
    U8 used_threads = 0;
    pthread_t threads[t->amount_of_threads];
    U8 players_threads_i[t->amount_of_threads];
    for (U8 i = 0; i < t->amount_of_players; i++) {
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
        pthread_create(
            &threads[used_threads], NULL,
            birth, heap_name
        );
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
                    players_threads_i[i1] + 1, t->amount_of_players,
                    t->players[players_threads_i[i1]].name
                );
            }
            used_threads = 0;
        }
    }
}

U0 *simulate_game(U0 *simargs) {
    RunningGame *game = simargs;

    struct {PF function; U0 *args;} pfs[2];

    for (U8 i = 0; i < 2; i++) {
        TPlayer *player = &game->t->players[game->p[i]];
        if (player->type == NN_TPlayer) {
            pfs[i].function = neural_network;
            pfs[i].args = (U0 *)&player->arg.nn;
        } else {
            pfs[i].function = player->arg.bot;
        }
    }

    uintptr_t winner = play(bg,
        pfs[0].function, pfs[0].args,
        pfs[1].function, pfs[1].args
    );

    for (U8 i = 0; i < 2; i++) {
        TPlayer *player = &game->t->players[game->p[i]];
        atomic_U8 *in_use = &player->in_use;
        atomic_store(in_use, atomic_load(in_use) - 1);
        if (
            player->type == NN_TPlayer &&
            atomic_load(in_use) <= 0
        ) {
            close_nn(player->arg.nn);
            free(player->arg.nn);
        }
    }

    atomic_store(&game->done, true);

    return (U0 *)winner;
}

static U0 play_round(Tournament *t) {
    t->round_count++;
    char title_text[20];
    snprintf(title_text, 20, "round %d:", t->round_count);
    title(title_text);
    U32 total_games = 0;
    for (U8 i = 0; i < t->amount_of_players; i++) {
        total_games += (t->amount_of_players - i - 1) * 2;
    }

    for (U8 i = 0; i < t->amount_of_players; i++) {
        t->players[i].points = 0;
        atomic_store(&t->players[i].in_use, 0);
    }
    
    RunningGame all_games[total_games];

    {
        U16 i = 0;
        for (U8 i1 = 0; i1 < t->amount_of_players; i1++) {
            for (U8 i2 = i1 + 1; i2 < t->amount_of_players; i2++) {
                for (U8 color = 0; color < 2; color++) {
                    RunningGame game = (RunningGame){.t = t};
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
        pthread_create(
            &all_games[i].thread, NULL, simulate_game, &all_games[i]
        );
    }

    for (U32 i = t->amount_of_threads; i < total_games - t->amount_of_threads; i++) {
        bool found = false;
        while (!found) {
            for (U8 i2 = 0; i2 < i; i2++) {
                if (atomic_load(&all_games[i2].done)) {
                    U8 winner;
                    pthread_join(all_games[i2].thread, (U0 *)&winner);

                    pthread_create(
                        &all_games[i].thread, NULL, simulate_game, &all_games[i]
                    );
                    found = true;
                }
            }
        }
    }
}

static U0 open_tplayers(Tournament *t) {
    
}

static int compare_tplayers(const U0 *a, const U0 *b) {
    U32 ap = ((TPlayer *)a)->points;
    U32 bp = ((TPlayer *)b)->points;
    return (ap > bp) - (ap < bp);
}

static U0 sort_players(Tournament *t) {
    qsort(
        t->players, t->amount_of_players,
        sizeof(TPlayer), compare_tplayers
    );
    title("ranking");
    for (U8 i = 0; i < 10 && i < t->amount_of_players; i++) {
        printf(
            "%d. \033[32m%s\033[0m (%d)\n", i + 1,
            t->players[i].name, t->players[i].points
        );
    }
}

U0 tournament(U8 amount_of_threads) {
    srand(time(NULL));

    Tournament t = {
        .amount_of_threads = amount_of_threads,
        .amount_of_players = 3
    };

    initial_birth(&t);

    play_round(&t);

    sort_players(&t);
    
    title("end");
    free(t.players);
}

