#include "chess.h"

static U8 ollama(const Game *game, U0 *model) {
    U16 history_len = 0;
    for (U8 i = 0; i < game->amount_of_moves; i++) {
        history_len += strlen(game->moves[i]) + 1;
    }
    U8 index = 0;
    char *history = malloc(history_len);
    for (U8 i = 0; i < game->amount_of_moves; i++) {
        for (U8 i2 = 0; i2 < strlen(game->moves[i]); i2++) {
            history[index + i2] = game->moves[i][i2];
        }
        history[strlen(game->moves[i]) + index] = ' ';
        index += strlen(game->moves[i]) + 1;
    }
    U16 legal_moves_len = 0;
    for (U8 i = 0; i < game->amount_of_legal_moves; i++) {
        legal_moves_len += strlen(game->legal_moves[i].notation) + 1;
    }
    index = 0;
    char *legal_moves = malloc(legal_moves_len);
    for (U8 i = 0; i < game->amount_of_legal_moves; i++) {
        for (U8 i2 = 0; i2 < strlen(game->legal_moves[i].notation); i2++) {
            legal_moves[index + i2] = game->legal_moves[i].notation[i2];
        }
        legal_moves[strlen(game->legal_moves[i].notation) + index] = ' ';
        index += strlen(game->legal_moves[i].notation) + 1;
    }
    char command[1024];
    snprintf(
        command, sizeof(command),
        "ollama run %s \""
        "Your are playing chess. You are playing as %s.\n"
        "This is the entire game history: %s\n"
        "These are all the possible moves: %s\n"
        "Answer with your move only."
        "\" --hidethinking 2>/dev/null",
        (char *)model,
        game->turn == WHITE ? "white" : "black",
        history, legal_moves
    );
    free(history); free(legal_moves);
    FILE *pipe = popen(command, "r");
    char answer[MAX_MOVE_NOTATION_LEN];
    if (fgets(answer, sizeof(answer), pipe) != NULL) {
        answer[strlen(answer) - 1] = '\0';
        for (U8 i = 0; i < game->amount_of_legal_moves; i++) {
            if (strcmp(answer, game->legal_moves[i].notation) == 0) {
                return i;
            }
        }
    }
    //fprintf(stderr, "illegal move by ollama. prompt: [%s] answer: [%s]\n", command, answer);
    return ollama(game, model);
}