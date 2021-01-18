//
// Created by dszhdankin on 13.01.2021.
//

#ifndef NAVY_COMBAT_SERVER_H
#define NAVY_COMBAT_SERVER_H
#include <memory>
#include <mutex>
#include <atomic>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <poll.h>
#include <netinet/in.h>
#include <thread>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include "../utils/ILogger.h"
#include "../utils/StraightforwardLogger.h"
#include "Player.h"
#include <condition_variable>
#include <sys/resource.h>
#include <algorithm>

enum InitializationStatus {
    NOT_INITIALIZED = 0,
    SUCCESS,
    FAILED
};

//TODO implement navy combat server
//TODO fix listener and initializer threads
//TODO implement game finder and game worker threads
class Server {
private:
    int MAX_GAME_SESSIONS;
    int MAX_TOTAL_CLIENTS;
    int FD_LIMIT;

    volatile int _cur_game_sessions;
    volatile int _cur_waiting_for_game;
    volatile int _cur_initializing;
    volatile int _cur_total_clients;

    int _epoll_game_read_fd;
    int _epoll_game_write_fd;

    int _epoll_initializer_read_fd;
    int _epoll_initializer_write_fd;

    int _epoll_finder_write_fd;

    int _polls_timeout;

    int _listen_fd;
    std::unique_ptr<std::thread> _listener_thread;

    std::vector<char> _in_initializer_pool;
    std::vector<std::shared_ptr<Player>> _initialized_player;
    std::vector<InitializationStatus> _initialization_status;
    std::unique_ptr<std::thread> _initializer_thread;

    std::mutex _pool_game_mtx;
    std::mutex _pool_initializer_mtx;
    std::mutex _pool_finder_mtx;
    std::mutex _total_clients_mtx;

    std::condition_variable _listener_cond_var;
    std::condition_variable _game_finder_cond_var;

    std::atomic<bool> _shutdown;

    std::unique_ptr<ILogger> _logger;

    void Listener();
    void putIntoInitializerPool(int clientFd);

    void Initializer();
    void removeFromInitializerPool(int clientFd, bool notify, bool deregisterOnly = false);
    void initializerFreeResources();

    //Sleeps on _pool_game_mtx
    void GameFinder();

    void GameWorker();

public:
    Server();
    //Might need to wait until the system cleans up the port
    void start(uint16_t port, int backlog, int maxTotalClients, int maxGameSessions, int pollsTimeout);
    void shutdown();
    ~Server() {};
};

Server::Server(): _pool_finder_mtx(), _pool_initializer_mtx(), _pool_game_mtx(),
    _logger(new StraightforwardLogger(std::cout)), MAX_GAME_SESSIONS(-1),
    MAX_TOTAL_CLIENTS(-1), _cur_game_sessions(-1), _cur_waiting_for_game(-1),
    _cur_total_clients(-1), _epoll_finder_write_fd(-1), _epoll_game_read_fd(-1), _epoll_game_write_fd(-1),
    _epoll_initializer_read_fd(-1), _epoll_initializer_write_fd(-1), _shutdown(true), _listen_fd(-1),
    _listener_cond_var(), _total_clients_mtx(), _game_finder_cond_var(), _polls_timeout(-1),
    _listener_thread(nullptr), _initializer_thread(nullptr){

    rlimit limits;
    //Maybe one day I will find what to do with error in getting limit)
    getrlimit(RLIMIT_NOFILE, &limits);
    FD_LIMIT = limits.rlim_cur + 10;

    _in_initializer_pool.resize(FD_LIMIT, false);
    _initialized_player.resize(FD_LIMIT, std::shared_ptr<Player>(nullptr));
    _initialization_status.resize(FD_LIMIT, InitializationStatus::NOT_INITIALIZED);
}

void Server::start(uint16_t port, int backlog, int maxTotalClients, int maxGameSessions, int pollsTimeout) {
    MAX_TOTAL_CLIENTS = maxTotalClients;
    MAX_GAME_SESSIONS = maxGameSessions;
    _polls_timeout = pollsTimeout;

    _cur_total_clients = _cur_initializing = 0;

    _shutdown.store(false);

    /*Prepare listener thread*/
    _listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_listen_fd < 0) {
        _logger->saveLog("Cannot create listening socket! Start failed!");
        return;
    }
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(_listen_fd, (sockaddr *) &addr, sizeof (addr)) < 0) {
        _logger->saveLog("Cannot bind listening socket to port! Start failed!");
        close(_listen_fd);
        return;
    }

    if (listen(_listen_fd, backlog) < 0) {
        _logger->saveLog("Cannot start listening! Start failed!");
        close(_listen_fd);
        return;
    }
    /*Prepare listener thread*/

    /*Prepare initializer thread*/
    std::fill(_in_initializer_pool.begin(), _in_initializer_pool.end(), false);
    std::fill(_initialized_player.begin(), _initialized_player.end(), nullptr);
    std::fill(_initialization_status.begin(), _initialization_status.end(),
              InitializationStatus::NOT_INITIALIZED);

    _epoll_initializer_read_fd = epoll_create(FD_LIMIT);
    if (_epoll_initializer_read_fd < 0) {
        _logger->saveLog("Cannot create epoll read for initializer! Start failed!");
        close(_listen_fd);
        return;
    }

    _epoll_initializer_write_fd = epoll_create(FD_LIMIT);
    if (_epoll_initializer_write_fd < 0) {
        _logger->saveLog("Cannot create epoll write for initializer! Start failed!");
        close(_listen_fd);
        return;
    }
    /*Prepare initializer thread*/

    _listener_thread = std::make_unique<std::thread>(&Server::Listener, this);
    _initializer_thread = std::make_unique<std::thread>(&Server::Initializer, this);

    _logger->saveLog("Server started!");
}

void Server::shutdown() {

    {
        //We have to wake up Listener;
        std::lock_guard<std::mutex> lock(_total_clients_mtx);
        _shutdown.store(true);
        _listener_cond_var.notify_one();
    }

    _listener_thread->join();
    _logger->saveLog("Listener shut down!");
    _initializer_thread->join();

    _logger->saveLog("Server shut down!");
    //TODO notification for game finder
}

//Sleeps on _total_clients_mtx
void Server::Listener() {
    pollfd pfd[1];
    pfd[0].fd = _listen_fd;
    pfd[0].events = POLLIN;

    sockaddr_in clientAddr;
    socklen_t size=sizeof(struct sockaddr_in);

    while (true) {
        {
            std::unique_lock<std::mutex> lock(_total_clients_mtx);
            //We can sleep here
            if (_cur_total_clients >= MAX_TOTAL_CLIENTS && !_shutdown.load()) {
                _listener_cond_var.wait(lock, [this] () {
                    return _cur_total_clients < MAX_TOTAL_CLIENTS || _shutdown.load();
                });
            }
        }

        if (_shutdown.load()) {
            close(_listen_fd);
            return;
        }

        int canAccept = poll(pfd, 1, _polls_timeout);
        if (canAccept < 0) {
            _logger->saveLog("Poll failed in listener thread! Listener thread shutdowns!");
            close(_listen_fd);
            return;
        } else if (canAccept > 0) {
            int clientFd = accept(_listen_fd, (sockaddr *) &clientAddr, &size);
            if (clientFd < 0) {
                _logger->saveLog("Accept failed in listener thread! Listener thread shutdowns!");
                close(_listen_fd);
                return;
            } else {
                putIntoInitializerPool(clientFd);
            }
        }

    }
}

void Server::putIntoInitializerPool(int clientFd) {
    epoll_event event;
    int flags = fcntl(clientFd, F_GETFL, 0);
    fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);

    {
        std::lock_guard<std::mutex> lock(_total_clients_mtx);
        _cur_total_clients++;
    }

    {
        std::lock_guard<std::mutex> lock(_pool_initializer_mtx);
        _cur_initializing++;
        _in_initializer_pool[clientFd] = true;
    }

    event.data.fd = clientFd;
    event.events = EPOLLIN | EPOLLHUP | EPOLLERR;
    epoll_ctl(_epoll_initializer_read_fd, EPOLL_CTL_ADD, clientFd, &event);

    event.data.fd = clientFd;
    event.events = EPOLLOUT | EPOLLHUP | EPOLLERR;
    epoll_ctl(_epoll_initializer_write_fd, EPOLL_CTL_ADD, clientFd, &event);
}

void Server::Initializer() {
    std::unique_ptr<epoll_event[]> events(new epoll_event[MAX_TOTAL_CLIENTS]);

    while (true) {
        if (_shutdown.load()) {
            initializerFreeResources();
            return;
        }

        int readNum = epoll_wait(_epoll_initializer_read_fd, events.get(), MAX_TOTAL_CLIENTS, _polls_timeout);
        if (readNum < 0) {
            _logger->saveLog("Epoll read failed in initializer thread! Initializer thread shutdowns!");
            initializerFreeResources();
            return;
        } else {
            for (int i = 0; i < readNum; i++) {
                int curClientFd = events[i].data.fd;
                if ((events[i].events & EPOLLHUP) || (events[i].events & EPOLLERR)) {
                    removeFromInitializerPool(curClientFd, true);
                } else if ((events[i].events & EPOLLIN) &&
                    (_initialization_status[curClientFd] == InitializationStatus::NOT_INITIALIZED)) {
                    _initialized_player[curClientFd].reset(new Player(curClientFd));
                    _initialization_status[curClientFd] = InitializationStatus::SUCCESS;
                    if (_initialized_player[curClientFd]->getToken() != "greetings")
                        _initialization_status[curClientFd] = InitializationStatus::FAILED;
                    else
                        _logger->saveLog("Greetings success!");
                    for (int i = 0; i < 10; i++) {
                        if (!_initialized_player[curClientFd]->readShip()) {
                            _initialization_status[curClientFd] = InitializationStatus::FAILED;
                            break;
                        } else {
                            std::string msg("0-th ship success!");
                            msg[0] += i;
                            _logger->saveLog(msg);
                        }
                    }
                }
            }
        }

        int writeNum = epoll_wait(_epoll_initializer_write_fd, events.get(), MAX_TOTAL_CLIENTS, _polls_timeout);
        if (writeNum < 0) {
            _logger->saveLog("Epoll write failed in initializer thread! Initializer thread shutdowns!");
            initializerFreeResources();
            return;
        } else {
            for (int i = 0; i < writeNum; i++) {
                int curClientFd = events[i].data.fd;
                if ((events[i].events & EPOLLHUP) || (events[i].events & EPOLLERR)) {
                    removeFromInitializerPool(curClientFd, true);
                } else if ((events[i].events & EPOLLOUT) &&
                    (_initialization_status[curClientFd] == InitializationStatus::SUCCESS)) {
                    std::shared_ptr<Player> player = _initialized_player[curClientFd];
                    player->writeTokens(player->getShips());
                    removeFromInitializerPool(curClientFd, true);
                    //TODO move to finder pool
                } else if ((events[i].events & EPOLLOUT) &&
                    (_initialization_status[curClientFd] == InitializationStatus::FAILED)) {
                    std::shared_ptr<Player> player = _initialized_player[curClientFd];
                    player->writeTokens("incorrect_greeting;");
                    removeFromInitializerPool(curClientFd, true);
                }
            }
        }
    }
}

void Server::removeFromInitializerPool(int clientFd, bool notify, bool deregisterOnly) {

    epoll_event evt;
    epoll_ctl(_epoll_initializer_read_fd, EPOLL_CTL_DEL, clientFd, &evt);
    epoll_ctl(_epoll_initializer_write_fd, EPOLL_CTL_DEL, clientFd, &evt);
    if (!deregisterOnly)
        close(clientFd);

    {
        std::lock_guard<std::mutex> lock(_pool_initializer_mtx);
        _in_initializer_pool[clientFd] = false;
        _initialization_status[clientFd] = InitializationStatus::NOT_INITIALIZED;
        _initialized_player[clientFd] = nullptr;
        _cur_initializing--;
    }

    {
        std::lock_guard<std::mutex> lock(_total_clients_mtx);
        _cur_total_clients--;
        if (notify)
            _listener_cond_var.notify_one();
    }
}

void Server::initializerFreeResources() {
    for (int clientFd = 0; clientFd < _in_initializer_pool.size(); clientFd++) {
        if (_in_initializer_pool[clientFd]) {
            removeFromInitializerPool(clientFd, false);
        }
    }

    close(_epoll_initializer_read_fd);
    close(_epoll_initializer_write_fd);
}


#endif //NAVY_COMBAT_SERVER_H
