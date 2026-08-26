#include "chess.h"
#include "tui.h"
#include "bots.h"
#include "nn.h"

struct Player {
    char *name;
    PF function;
    U0 *args;
    char *description;
};

static const struct Player players[] = {
    {"human", human, NULL},
    {"jonkler", jonkler, NULL, "a bot that makes random moves"},
    {"random", jonkler, NULL, "alias to jonkler"},
    {"thief", thief, NULL, "\n"
        "      a bot that makes random moves but prefers moves\n"
        "      where a piece as valuable as possible is captured"
    },
    {"murderer", murderer, NULL, "\n"
        "      a bot that makes random moves but prefers moves\n"
        "      where the king is attacked"
    },
    {"stockfih", stockfih, NULL, "a smart and predicting chess bot"
    },
    {"gpt-5.5", codex, "gpt-5.5",
        "requires codex cli to be installed an set up"
    },
    {"gemma4:e2b", ollama, "gemma4:e2b", "\n"
        "      requires ollama to be installed and the \n"
        "      model gemma4:e2b to be downloaded"
    },
    {"gemma4:e2b-mlx", ollama, "gemma4:e2b-mlx", "\n"
        "      requires ollama to be installed and the \n"
        "      model gemma4:e2b-mlx to be downloaded"
    }
};

static const U32 amount_of_players = (
    sizeof(players) / sizeof(struct Player)
);

static U0 help() {
    printf(
        "usage: chess <player | path> <player | path> [--bg] [--benchmark]\n\n"
        "<player> options:\n"
    );
    for (U32 p = 0; p < amount_of_players; p++) {
        printf("   %s", players[p].name);
        if (players[p].description != NULL) {
            printf(": %s", players[p].description);
        }
        printf("\n");
    }
    printf(
        "\n"
        "<path>:\n"
        "   path to a .nn file\n\n"
        "--bg:\n"
        "   run the game in the background (can not be used with human)\n\n"
        "--benchmark:\n"
        "   let the players rematch forever while counting their wins\n"
    );
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    if (argc > 2 && strcmp(argv[1], "tournament") == 0) {
        tournament(atoi(argv[2]));
        return 0;
    }

    struct Player selected_players[2];
    U8 count_selected = 0;
    bool run_in_bg = false;
    bool benchmark = false;
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
            } else if (strcmp(argv[i], "--benchmark") == 0) {
                run_in_bg = true;
                benchmark = true;
            } else if (strcmp(argv[i] + arg_len - 3, ".nn") == 0) {
                NN *nn = open_nn(argv[i]);
                for (U32 i2 = strlen(argv[i]); i2 > 1; i2--) {
                    if (argv[i][i2 - 1] == '/') {
                        selected_players[count_selected].name = argv[i] + i2;
                        break;
                    }
                }
                U8 name_len = strlen(selected_players[count_selected].name);
                //selected_players[count_selected].name[name_len - 3] = '\0';
                selected_players[count_selected].function = neural_network;
                selected_players[count_selected].args = nn;
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
        bool playing_human = (
            selected_players[0].function == human ||
            selected_players[1].function == human
        );
        if (benchmark) {
            if (playing_human) {
                run_in_bg = false;
            }
            U8 switch_players = 0;
            U32 results[3] = {0};
            bool first = true;
            bool same = strcmp(
                selected_players[0].name, selected_players[1].name
            ) == 0;
            char *color_names[] = {"white", "black"};
            while (true) {
                if (first || playing_human) first = false; else {
                    printf("\033[3F");
                }
                U8 max_name_len = 4;
                U8 name_lens[2];
                for (U8 p = 0; p < 2; p++) {
                    name_lens[p] = strlen(
                        same ? color_names[p] : selected_players[p].name
                    );
                }
                if (name_lens[0] > max_name_len) max_name_len = name_lens[0];
                if (name_lens[1] > max_name_len) max_name_len = name_lens[1];
                printf("draw: ");
                for (U8 i = 0; i < max_name_len - 4; i++) printf(" ");
                printf("%d\n", results[0]);
                for (U8 p = 0; p < 2; p++) {
                    printf("%s: ",
                        same ? color_names[p] : selected_players[p].name
                    );
                    for (U8 i = 0; i < max_name_len - name_lens[p]; i++) {
                        printf(" ");
                    }
                    printf("%d\n", results[p + 1]);
                }
                U8 winner = play(run_in_bg ? bg : tui,
                    selected_players[switch_players].function,
                    selected_players[switch_players].args,
                    selected_players[1 - switch_players].function,
                    selected_players[1 - switch_players].args
                );
                if (winner == 0) {
                    results[0]++;
                } else {
                    if (switch_players == 0) {
                        results[winner]++;
                    } else {
                        results[1 - (winner - 1) + 1]++;
                    }
                }
                if (!same) switch_players = 1 - switch_players;
            }
        } else {
            if (playing_human && run_in_bg) {
                help();
                return 1;
            }
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
            return 0;
        }
    }
    help();
    return 1;
}

