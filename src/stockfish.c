#include <ctype.h>
#include <inttypes.h>

#include "chess.h"

static const char abc[] = ABC;

char *get_stockfish_notation(Move move) {
    char *eq = strchr(move.notation, '=');
    bool is_promotion = eq != NULL;
    char promotion_piece = is_promotion ? eq[1] : 0;
    char *result = malloc((5 + is_promotion) * sizeof(char));
    if (result == NULL) out_of_mem();
    result[0] = abc[move.start.x];
    result[1] = '0' + (move.start.y + 1);
    result[2] = abc[move.end.x];
    result[3] = '0' + (move.end.y + 1);
    if (is_promotion) {
        result[4] = (char)tolower((unsigned char)promotion_piece);
        result[5] = '\0';
    } else {
        result[4] = '\0';
    }
    return result;
}

char *get_game_history_in_stockfish_notation(const Game *game) {
    U32 result_len = 0;
    for (U16 i = 0; i < game->amount_of_moves; i++) {
        result_len += strchr(game->moves[i], '=') == NULL ? 6 : 7;
    }
    char *result = malloc((result_len + 1) * sizeof(char));
    if (result == NULL) out_of_mem();
    U32 index = 0;
    Game test_game = new_game();
    for (U16 i = 0; i < game->amount_of_moves; i++) {
        for (U16 lm = 0; lm < test_game.amount_of_legal_moves; lm++) {
            if (strcmp(test_game.legal_moves[lm].notation, game->moves[i]) == 0) {
                char *move_sf_not = get_stockfish_notation(test_game.legal_moves[lm]);
                for (U8 x = 0; x < strlen(move_sf_not); x++) {
                    result[index] = move_sf_not[x];
                    index++;
                }
                result[index] = ' ';
                index++;
                free(move_sf_not);
                break;
            }
        }
        do_move(&test_game, game->moves[i]);
    }
    result[index > 0 ? index - 1 : 0] = '\0';
    close_game(&test_game);
    return result;
}

char *ask_stockfish(const Game *game, U32 time) {
    bool startpos = game->amount_of_moves <= 0;
    char *history = startpos ? "\0" : get_game_history_in_stockfish_notation(game);
    char command[65536];
    snprintf(
        command, sizeof(command),
        "{\n"
        "echo 'position startpos moves %s'\n"
        "echo 'go movetime %u'\n"
        "sleep %f\n"
        "echo 'quit'\n"
        "} | stockfish | grep ^bestmove",
        history, time, ((float)time + 100.0f) / 1000.0f
    );
    if (!startpos) free(history);
    FILE *pipe = popen(command, "r");
    if (pipe == NULL) {
        fprintf(stderr,
            "stockfish could not move"
            " because pipe could not be opened\n"
        );
        exit(1);
    }
    char *command_answer = malloc(64 * sizeof(char));
    if (command_answer == NULL) out_of_mem();
    if (fgets(command_answer, 64, pipe) != NULL) {
        command_answer[strcspn(command_answer, "\r\n")] = '\0';
    }
    pclose(pipe);
    return command_answer;
}

U8 stockfish(const Game *game, U0 *time) {
    char *sf_answer = ask_stockfish(game, (uintptr_t)time);
    char *answer = strstr(sf_answer, "bestmove") + 9;
    if (strchr(answer, ' ') != NULL) answer[strchr(answer, ' ') - answer] = '\0';
    for (U8 i = 0; i < game->amount_of_legal_moves; i++) {
        char *stockfish_notation = get_stockfish_notation(game->legal_moves[i]);
        if (strcmp(answer, stockfish_notation) == 0) {
            free(stockfish_notation);
            free(sf_answer);
            return i;
        }
        free(stockfish_notation);
    }
    free(sf_answer);
    fprintf(stderr, "illegal move by stockfish\n");
    exit(1);
    return 0;
}
