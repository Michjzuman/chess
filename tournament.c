#include <pthread.h>
#include <stdatomic.h>

#include "chess.h"
#include "bots.h"
#include "nn.h"
#include "tui.h"

#define max_name_len 9
#define max_path_len 21

typedef atomic_int_least8_t atomic_U8;

typedef struct {
    char name[max_name_len];
    U32 points;
} TPlayer;

typedef struct {
    TPlayer *players[2];
} TRound;

typedef struct {
    U8 amount_of_threads;
    U8 amount_of_players;
    TPlayer *players;
    U32 round_count;
} Tournament;

typedef struct {
    NN *nns;
    atomic_bool *thread_state;
    atomic_U8 *count[2];
} SimArgs;

static char *get_path(char *name) {
    char *path = malloc(max_path_len * sizeof(char));
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
    U8 used_threads = 0;
    pthread_t *threads = malloc(
        t->amount_of_threads * sizeof(pthread_t)
    );
    U8 *players_threads_i = malloc(t->amount_of_threads * sizeof(U8));
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
    free(players_threads_i);
    free(threads);
}

U0 *simulate_game(U0 *simargs) {
    SimArgs *args = simargs;
    /*
    printf("=== [%p] === (start)\n", args);
    */
    uintptr_t winner = play(bg,
        neural_network, &args->nns[0],
        neural_network, &args->nns[1]
    );
    atomic_store(args->count[0], atomic_load(args->count[0]) - 1);
    atomic_store(args->count[1], atomic_load(args->count[1]) - 1);
    /*
    printf("=== [%p] === (end)\n", args);
    */
    if (atomic_load(args->count[0]) <= 0) {
        close_nn(&args->nns[0]);
        free(args->count[0]);
    }
    if (atomic_load(args->count[1]) <= 0) {
        close_nn(&args->nns[1]);
        free(args->count[1]);
    }
    free(args->nns);
    atomic_store(args->thread_state, false);
    free(args);
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
    U32 total_game_count = 0;

    for (U8 i = 0; i < t->amount_of_players; i++) {
        t->players[i].points = 0;
    }
    U8 used_threads = 0;
    U8 peak_threads = 0;
    pthread_t *threads = malloc(t->amount_of_threads * sizeof(pthread_t));
    TRound *players_on_thread = malloc(
        t->amount_of_threads * sizeof(TRound)
    );
    atomic_bool *thread_states = malloc(
        t->amount_of_threads * sizeof(atomic_bool)
    );

    U8 next_thread = 0;

    NN *nns[2];
    for (U8 i1 = 0; i1 < t->amount_of_players; i1++) {
        {
            char *path = get_path(t->players[i1].name);
            nns[0] = malloc(sizeof(NN));
            *nns[0] = open_nn(path);
            free(path);
        }
        atomic_U8 *count[2];
        count[0] = malloc(sizeof(atomic_U8));
        atomic_store(count[0], (t->amount_of_players - i1 - 1) * 2);
        for (U8 i2 = i1 + 1; i2 < t->amount_of_players; i2++) {
            {
                char *path = get_path(t->players[i2].name);
                nns[1] = malloc(sizeof(NN));
                *nns[1] = open_nn(path);
                free(path);
            }
            count[1] = malloc(sizeof(atomic_U8));
            atomic_store(count[1], 2);
            for (U8 color = 0; color < 2; color++) {
                U8 p[2] = {color == 0 ? i1 : i2, color == 0 ? i2 : i1};
                {
                    SimArgs *args = malloc(sizeof(SimArgs));
                    args->nns = malloc(2 * sizeof(NN));
                    args->nns[0] = *nns[0];
                    args->nns[1] = *nns[1];
                    args->thread_state = &thread_states[next_thread];
                    args->count[0] = count[0];
                    args->count[1] = count[1];
                    pthread_create(
                        &threads[next_thread], NULL,
                        simulate_game, args
                    );
                    players_on_thread[next_thread].players[0] = &t->players[p[0]];
                    players_on_thread[next_thread].players[1] = &t->players[p[1]];
                    used_threads++;
                    if (peak_threads < used_threads) {
                        peak_threads = used_threads;
                    }
                    atomic_store(&thread_states[next_thread], true);
                }
                while (used_threads > 0) {
                    next_thread++;
                    next_thread %= t->amount_of_threads;
                    bool last = (
                        i1 == t->amount_of_players - 2 &&
                        i2 == t->amount_of_players - 1 &&
                        color == 1
                    );
                    if (!atomic_load(&thread_states[next_thread]) || last) {
                        if (peak_threads >= t->amount_of_threads) {
                            U0 *response;
                            pthread_join(threads[next_thread], &response);
                            used_threads--;
                            total_game_count++;
                            U8 winner = (uintptr_t)response;
                            printf(
                                "%d/%d: \033[32m%s\033[0m vs. \033[32m%s\033[0m: ",
                                total_game_count, total_games,
                                players_on_thread[next_thread].players[0]->name,
                                players_on_thread[next_thread].players[1]->name
                            );
                            if (winner == 0) {
                                printf("\033[33mdraw");
                                players_on_thread[next_thread].players[0]->points++;
                                players_on_thread[next_thread].players[1]->points++;
                            } else {
                                printf(
                                    "\033[32m%s\033[0m \033[36mwon!",
                                    players_on_thread[next_thread].players[winner - 1]->name
                                );
                                players_on_thread[next_thread].players[winner - 1]->points += 2;
                            }
                            printf("\033[0m\n");
                        }
                        if (!last) break;
                    }
                }
            }
        }
    }
    free(players_on_thread);
    free(thread_states);
    free(threads);
}

static U0 open_players(Tournament *t) {

}

static U0 sort_players(Tournament *t) {
    for (U8 i = 0; i < t->amount_of_players; i++) {
        printf(
            "%s: %d\n",
            t->players[i].name,
            t->players[i].points
        );
    }
}

U0 tournament(U8 amount_of_threads) {
    srand(time(NULL));

    Tournament t = {
        .amount_of_threads = amount_of_threads,
        .amount_of_players = 50
    };

    initial_birth(&t);

    play_round(&t);

    sort_players(&t);
    
    title("end");
    free(t.players);
}

