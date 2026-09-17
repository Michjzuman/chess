#include "chess.h"
#include "tui.h"
#include "bots.h"
#include "nn.h"
#include "pgn.h"

struct Player {
    char *name;
    PF function;
    U0 *args;
    char *description;
};

static const struct Player players[] = {
    {"human", human, NULL, "the tui interface for humans"},
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
    {"bot1", peak_bot, (U0 *)1},
    {"bot2", peak_bot, (U0 *)2},
    {"bot3", peak_bot, (U0 *)3},
    {"bot4", peak_bot, (U0 *)4},
    {"bot5", peak_bot, (U0 *)5},
    {"bot6", peak_bot, (U0 *)6},
    {"bot7", peak_bot, (U0 *)7, "\n"
        "      a bot that predicts <1-7> moves and foresees\n"
        "      wins and losses. if he sees none, he moves randomly"
    },
    {"peak_bot", peak_bot, NULL, "\n"
        "      the PERFECT chess bot that predicts EVERY move\n"
        "      (takes a while)"
    },
    {"gpt-6-astra", codex, "gpt-6-astra"},
    {"gpt-5.6-sol", codex, "gpt-5.6-sol"},
    {"gpt-5.6-terra", codex, "gpt-5.6-terra"},
    {"gpt-5.6-luna", codex, "gpt-5.6-luna", "\n"
        "      require codex cli to be installed an set up"
    },
    {"qwen3.5:4b-mxfp8", ollama, "qwen3.5:4b-mxfp8"},
    {"qwen3.5:9b-mxfp8", ollama, "qwen3.5:9b-mxfp8"},
    {"gemma3:270m", ollama, "gemma3:270m"},
    {"gemma3:1b", ollama, "gemma3:1b"},
    {"gemma4:e2b", ollama, "gemma4:e2b"},
    {"gemma4:e2b-mlx", ollama, "gemma4:e2b-mlx", "\n"
        "      require ollama to be installed and the\n"
        "      individual models to be downloaded"
    },
    {"stockfish-fast", stockfish, (U0 *)1000},
    {"stockfish", stockfish, (U0 *)10000,
        "require stockfish to be installed"
    }
};

static const U32 amount_of_players = (
    sizeof(players) / sizeof(struct Player)
);

static U0 help() {
    printf(
        "usage: \n"
        "   chess <player | path> <player | path>\n"
        "         [--bg] [--benchmark] [--pgn | -o <path>]\n\n"
        "<player> options:\n"
        "   <your-name>\n"
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
        "   let the players rematch forever while counting their wins\n\n"
        "--pgn:\n"
        "   export the game as a pgn file\n\n"
        "-o <path>:\n"
        "   export the game as a pgn file to <path>\n"
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
    bool pgn = false;
    char *pgn_path = NULL;
    bool expect_pgn_path = false;

    for (U16 i = 1; i < argc; i++) {
        U8 arg_len = strlen(argv[i]);
        if (expect_pgn_path) {
            pgn_path = argv[i];
            expect_pgn_path = false;
        } else if (strcmp(argv[i], "--help") == 0) {
            help();
            return 0;
        } else if (strcmp(argv[i], "--bg") == 0) {
            run_in_bg = true;
        } else if (strcmp(argv[i], "--pgn") == 0) {
            pgn = true;
        } else if (strcmp(argv[i], "-o") == 0) {
            pgn = true;
            expect_pgn_path = true;
        } else if (strcmp(argv[i], "--benchmark") == 0) {
            run_in_bg = true;
            benchmark = true;
        } else if (
            count_selected < 2 && arg_len >= 3 &&
            strcmp(argv[i] + arg_len - 3, ".nn") == 0
        ) {
            NN *nn = open_nn(argv[i]);
            if (nn == NULL) {
                fprintf(stderr, "could not open neural network %s\n", argv[i]);
                return 1;
            }
            char *slash = strrchr(argv[i], '/');
            selected_players[count_selected].name = (
                slash == NULL ? argv[i] : slash + 1
            );
            selected_players[count_selected].function = neural_network;
            selected_players[count_selected].args = nn;
            count_selected++;
        } else if (count_selected < 2) {
            bool found = false;
            for (U16 p = 0; p < amount_of_players; p++) {
                if (strcmp(argv[i], players[p].name) == 0) {
                    selected_players[count_selected] = players[p];
                    found = true;
                    count_selected++;
                    break;
                }
            }
            if (!found && argv[i][0] != '-') {
                selected_players[count_selected] = (struct Player){
                    .function = human, .args = NULL,
                    .name = argv[i]
                };
                count_selected++;
            }
        } else {
            help();
            return 1;
        }
    }
    if (count_selected == 0) {
        selected_players[0] = players[0];
        selected_players[1] = players[0];
        count_selected = 2;
    }
    if (count_selected == 1) {
        U8 p1_color = rand() % 2;
        selected_players[p1_color] = selected_players[0];
        selected_players[1 - p1_color] = players[0];
        count_selected++;
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
                U8 winner = play_full(run_in_bg ? bg : tui,
                    selected_players[switch_players].function,
                    selected_players[switch_players].args,
                    selected_players[1 - switch_players].function,
                    selected_players[1 - switch_players].args,
                    pgn, &(PGNArgs){
                        .p1 = selected_players[switch_players].name,
                        .p2 = selected_players[1 - switch_players].name,
                        .path = pgn_path
                    }
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
            U8 winner = play_full(run_in_bg ? bg : tui,
                selected_players[0].function, selected_players[0].args,
                selected_players[1].function, selected_players[1].args,
                pgn, &(PGNArgs){
                    .p1 = selected_players[0].name,
                    .p2 = selected_players[1].name,
                    .path = pgn_path
                }
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
            for (U8 p = 0; p < 2; p++) {
                if (selected_players[p].function == neural_network) {
                    close_nn((NN *)selected_players[p].args);
                }
            }
            return 0;
        }
    }
    help();
    return 1;
}

