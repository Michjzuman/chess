#ifndef NET_H
#define NET_H

#include "chess.h"

U0 init_net_host(U16 local_port);
U0 init_net_join(char *server_ip, U16 server_port);
U0 close_net();

U8 net_local(const Game *, U0 *);
U8 net_peer(const Game *, U0 *);

#endif
