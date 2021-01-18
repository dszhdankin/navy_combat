#include "Server.h"
#include <sys/epoll.h>

Server::Server(int gameSessions): _epoll_mtx(), GAME_SESSIONS(gameSessions), _shutdown(true) {
    _epoll_fd = epoll_create(GAME_SESSIONS);
}