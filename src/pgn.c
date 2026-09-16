#include "pgn.h"

U0 create_pgn(const Game *game, const PGNArgs *args) {
    char *p1 = args->p1;
    char *p2 = args->p2;
    U8 result = args->result;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char default_path[512];
    snprintf(
        default_path, sizeof(default_path), "%s-vs-%s-%04d-%02d-%02d.pgn",
        p1, p2, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday
    );

    char *path = args->path == NULL ? default_path : args->path;

    FILE *file = fopen(path, "w");

    fprintf(file,
        "[Event \"%s vs %s\"]\n"
        "[Site \"https://github.com/michjzuman/chess\"]\n"
        "[Date \"%04d.%02d.%02d\"]\n"
        "[White \"%s\"]\n"
        "[Black \"%s\"]\n"
        "[Result \"%d-%d\"]\n"
        "\n",
        p1, p2,
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        p1, p2,
        result == 1, result == 2
    );

    U16 y = 0;
    for (U16 i = 0; i < game->amount_of_moves; i+=2) {
        y++;
        fprintf(file, "%u. %s", y, game->moves[i]);
        if (i + 1 < game->amount_of_moves) {
            fprintf(file, " %s", game->moves[i + 1]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

