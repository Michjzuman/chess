#include "chess.h"
#include "nn.h"
#include "tui.h"

#define amount_of_players 16
#define max_name_len 9
#define max_path_len 21

typedef struct {
    U16 points;
    char name[max_name_len];
} TPlayer;

U0 rand_name(U8 i, TPlayer *players) {
    bool found = false;
    while (!found) {
        static const char abc[] = (
            "abcdefghijklmnopqrstuvwxyz_"
        );
        U8 len = rand() % (max_name_len - 1) + 1;
        for (U8 i2 = 0; i2 < len; i2++) {
            players[i].name[i2] = abc[(U8)rand() % strlen(abc)];
        }
        players[i].name[len]= '\0';

        found = true;
        for (U8 i2 = 0; i2 < i; i2++) {
            if (strcmp(players[i].name, players[i2].name) == 0) {
                found = false;
            }
        }
    }
}

U0 tournament() {
    srand(time(NULL));

    TPlayer players[amount_of_players];

    for (U8 i = 0; i < amount_of_players; i++) {
        rand_name(i, players);
        char path[max_path_len];
        snprintf(
            path, max_path_len,
            "./brains/%s.nn", players[i].name
        );
        NN nn = new_chess_nn();
        save_nn(&nn, path);
        close_nn(&nn);
        printf(
            "%d/%d: \033[32m%s\033[0m was born!\n",
            i + 1, amount_of_players, players[i].name
        );
    }
    printf("\n");

    for (U8 i = 0; i < amount_of_players; i++) {
        players[i].points = 0;
    }
    for (U8 p1 = 0; p1 < amount_of_players; p1++) {
        for (U8 p2 = p1 + 1; p2 < amount_of_players; p2++) {
            for (U8 color = 0; color < 2; color++) {
                char names[2][max_name_len];
                strcpy(names[color], players[p1].name);
                strcpy(names[1 - color], players[p2].name);

                char paths[2][max_path_len];
                NN nns[2];
                for (U8 i = 0; i < 2; i++) {
                    snprintf(
                        paths[i], max_path_len,
                        "./brains/%s.nn", names[i]
                    );
                    nns[i] = open_nn(paths[i]);
                }

                printf("- - - - - - - - - - - - - - - - - -\n\n");

                for (U8 i = 0; i < amount_of_players; i++) {
                    printf(
                        "%s \033[32m%s\033[0m: %d\n",
                        (p1 == i || p2 == i) ? ">" : " ",
                        players[i].name, players[i].points
                    );
                }

                printf(
                    "\n- - - - - - - - - - - - - - - - - -\n"
                    "\n(\033[32m%s\033[0m) vs. (\033[32m%s\033[0m):\n",
                    names[0], names[1]
                );

                U8 winner = play(tui,
                    neural_network, &nns[0],
                    neural_network, &nns[1]
                );
                if (winner == 0) {
                    players[p1].points++;
                    players[p2].points++;
                    printf("\033[33mgame ended in a draw\033[0m\n\n");
                } else {
                    players[color == (winner - 1) ? p1 : p2].points += 2;
                    printf("\033[32m%s\033[0m won the game\n\n", names[(winner - 1)]);
                }

                for (U8 i = 0; i < 2; i++) {
                    close_nn(&nns[i]);
                }
            }
        }
    }


    /*
    for (U8 i = 0; i < amount_of_players; i++) {
        players[i] = (Player){"67master"};
    }

    char *path;
    snprintf(path, 20, "./brains/%s.nn", name);
    NN nn = open_nn("brains/test_brain.nn");

    play(tui, human, NULL, neural_network, &nn);
    
    close_nn(&nn);
    */
}

