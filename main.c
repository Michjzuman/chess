#include "chess.h"
#include "tui.h"
#include "bots.h"
#include "nn.h"

struct Player {
    char *name;
    PF function;
    U0 *args;
};

static const struct Player players[] = {
    {"human", human, NULL},
    {"jonkler", jonkler, NULL},
    {"thief", thief, NULL},
    {"murderer", murderer, NULL},
    {"gpt-5.5", codex, "gpt-5.5"},
    {"gemma4:e2b-mlx", ollama, "gemma4:e2b-mlx"}
};

static const U32 amount_of_players = (
    sizeof(players) / sizeof(struct Player)
);

static U0 help() {
    printf(
        "usage: chess <player | path> <player | path> [--bg]\n\n"
        "player:\n"
    );
    for (U32 p = 0; p < amount_of_players; p++) {
        printf("   %s\n", players[p].name);
    }
    printf(
        "\n"
        "path: \n"
        "   path to a .nn file\n\n"
        "--bg: \n"
        "   run the game in the background\n"
    );
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    if (argc > 2) {
        if (strcmp(argv[1], "training") == 0) {
            tournament(atoi(argv[2]));
            return 0;
        }
    }

    struct Player selected_players[2];
    U8 count_selected = 0;
    bool run_in_bg = false;
    if (argc == 1) {
        selected_players[0] = players[0];
        selected_players[1] = players[0];
        count_selected = 2;
    } else {
        for (U16 i = 1; i < argc; i++) {
            U8 arg_len = strlen(argv[i]);
            if (strcmp(argv[i], "--help") == 0) {
                help();
                return 0;
            } else if (strcmp(argv[i], "--bg") == 0) {
                run_in_bg = true;
            } else if (strcmp(argv[i] + arg_len - 3, ".nn") == 0) {
                NN nn = open_nn(argv[i]);
                for (U32 i2 = strlen(argv[i]); i2 > 1; i2--) {
                    if (argv[i][i2 - 1] == '/') {
                        selected_players[count_selected].name = argv[i] + i2;
                        break;
                    }
                }
                selected_players[count_selected].name[
                    strlen(selected_players[i - 1].name) - 3
                ] = '\0';
                selected_players[count_selected].function = neural_network;
                selected_players[count_selected].args = &nn;
                count_selected++;
            } else {
                bool found = false;
                for (U16 p = 0; p < amount_of_players; p++) {
                    if (strcmp(argv[i], players[p].name) == 0) {
                        selected_players[count_selected] = players[p];
                        found = true;
                        count_selected++;
                        break;
                    }
                }
                if (!found) return 1;
            }
        }
    }
    if (count_selected == 2) {
        U8 winner = play(run_in_bg ? bg : tui,
            selected_players[0].function, selected_players[0].args,
            selected_players[1].function, selected_players[1].args
        );
        if (winner == 0) {
            printf("draw\n");
        } else {
            printf("%s%s\033[0m won!\n",
                run_in_bg ? "" : winner == 1 ? "\033[32m" : "\033[34m",
                strcmp(
                    selected_players[0].name,
                    selected_players[1].name
                ) == 0 ?
                (char *[]){"white", "black"}[winner - 1] :
                selected_players[winner - 1].name
            );
        }
        for (U8 i = 0; i < 2; i++) {
            if (selected_players[i].function == neural_network) {
                close_nn(selected_players[i].args);
            }
        }
        return 0;
    }
    help();
    return 1;
}

