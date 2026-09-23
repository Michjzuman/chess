#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "chess.h"
#include "tui.h"

int sock;

U0 init_net_host(U16 local_port) {
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        perror("socket");
        exit(1);
    }
    int reuse = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(local_port),
        .sin_addr.s_addr = INADDR_ANY
    };
    if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }
    if (listen(server, 1) < 0) {
        perror("listen");
        exit(1);
    }
    sock = accept(server, NULL, NULL);
    if (sock < 0) {
        perror("accept");
        exit(1);
    }
    close(server);
}

U0 init_net_join(char *server_ip, U16 server_port) {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(server_port)
    };
    if (inet_pton(AF_INET, server_ip, &addr.sin_addr) != 1) {
        fprintf(stderr, "invalid server address: %s\n", server_ip);
        exit(1);
    }
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        exit(1);
    }
}

U0 close_net(U0) {
    close(sock);
}

U8 net_local(const Game *game, U0 *args) {
    U8 result = human(game, NULL);
    if (write(sock, &result, sizeof(U8)) != sizeof(U8)) {
        fprintf(stderr, "game disconnected\n");
        exit(1);
    }
    return result;
}

U8 net_peer(const Game *game, U0 *args) {
    U8 result;
    if (read(sock, &result, sizeof(U8)) != sizeof(U8)) {
        fprintf(stderr, "game disconnected\n");
        exit(1);
    }
    if (result >= game->amount_of_legal_moves) {
        fprintf(stderr, "Make sure both players have the same version\n");
        exit(1);
    }
    return result;
}
