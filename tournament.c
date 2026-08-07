#include <pthread.h>
#include <stdatomic.h>

#include "chess.h"
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

static U0 initial_birth(U8 amount_of_threads, U8 amount_of_players, TPlayer *players) {
    U8 used_threads = 0;
    pthread_t *threads = malloc(
        amount_of_threads * sizeof(pthread_t)
    );
    U8 *players_threads_i = malloc(amount_of_threads * sizeof(U8));
    for (U8 i = 0; i < amount_of_players; i++) {
        bool found = false;
        while (!found) {
            rand_name(players[i].name);
            found = true;
            for (U8 i2 = 0; i2 < i; i2++) {
                if (strcmp(players[i].name, players[i2].name) == 0) {
                    found = false;
                }
            }
        }
        char *heap_name = malloc(
            max_name_len * sizeof(char)
        );
        strcpy(heap_name, players[i].name);
        pthread_create(
            &threads[used_threads], NULL,
            birth, heap_name
        );
        players_threads_i[used_threads] = i;
        used_threads++;
        if (
            used_threads >= amount_of_threads ||
            i == amount_of_players - 1
        ) {
            for (U8 t = 0; t < used_threads; t++) {
                pthread_join(threads[t], NULL);
                printf(
                    "%d/%d: \033[32m%s\033[0m was \033[35mborn\033[0m!\n",
                    players_threads_i[t] + 1, amount_of_players,
                    players[players_threads_i[t]].name
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

static U0 play_round(U8 amount_of_threads, U8 amount_of_players, TPlayer *players) {
    U32 total_games = 0;
    for (U8 i = 0; i < amount_of_players; i++) {
        total_games += (amount_of_players - i - 1) * 2;
    }
    U32 total_game_count = 0;

    for (U8 i = 0; i < amount_of_players; i++) {
        players[i].points = 0;
    }
    U8 used_threads = 0;
    U8 peak_threads = 0;
    pthread_t *threads = malloc(amount_of_threads * sizeof(pthread_t));
    TPlayer (*players_on_thread)[2] = malloc(
        amount_of_threads * sizeof(*players_on_thread)
    );
    atomic_bool *thread_states = malloc(
        amount_of_threads * sizeof(atomic_bool)
    );

    U8 next_thread = 0;

    NN *nns[2];
    for (U8 i1 = 0; i1 < amount_of_players; i1++) {
        {
            char *path = get_path(players[i1].name);
            nns[0] = malloc(sizeof(NN));
            *nns[0] = open_nn(path);
            free(path);
        }
        atomic_U8 *count[2];
        count[0] = malloc(sizeof(atomic_U8));
        atomic_store(count[0], (amount_of_players - i1 - 1) * 2);
        for (U8 i2 = i1 + 1; i2 < amount_of_players; i2++) {
            {
                char *path = get_path(players[i2].name);
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
                    players_on_thread[next_thread][0] = players[p[0]];
                    players_on_thread[next_thread][1] = players[p[1]];
                    used_threads++;
                    if (peak_threads < used_threads) {
                        peak_threads = used_threads;
                    }
                    atomic_store(&thread_states[next_thread], true);
                }
                while (used_threads > 0) {
                    next_thread++;
                    next_thread %= amount_of_threads;
                    bool last = (
                        i1 == amount_of_players - 2 &&
                        i2 == amount_of_players - 1 &&
                        color == 1
                    );
                    if (!atomic_load(&thread_states[next_thread]) || last) {
                        if (peak_threads >= amount_of_threads) {
                            U0 *response;
                            pthread_join(threads[next_thread], &response);
                            used_threads--;
                            total_game_count++;
                            U8 winner = (uintptr_t)response;
                            printf(
                                "%d/%d: \033[32m%s\033[0m vs. \033[32m%s\033[0m: ",
                                total_game_count, total_games,
                                players_on_thread[next_thread][0].name,
                                players_on_thread[next_thread][1].name
                            );
                            if (winner == 0) {
                                printf("\033[33mdraw");
                            } else {
                                printf(
                                    "\033[32m%s\033[0m \033[36mwon!",
                                    players_on_thread[next_thread][winner - 1].name
                                );
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

U0 tournament(U8 amount_of_threads) {
    srand(time(NULL));

    U8 amount_of_players = 5;

    TPlayer *players = malloc(amount_of_players * sizeof(TPlayer));

    initial_birth(amount_of_threads, amount_of_players, players);

    play_round(amount_of_threads, amount_of_players, players);
    
    free(players);
}

