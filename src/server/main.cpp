#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <ctime>
#include <atomic>
#include <fstream>
#include <thread>
#include <functional>
#include "Server.h"

std::ofstream lg("log.txt");
std::atomic<bool> shut{false};

void communicate(int fd) {
    srand(time(0));
    char buf[15];
    char messages[4][30];

    memset(buf, 0, sizeof (buf));
    strcpy(messages[0], "Hello");
    strcpy(messages[1], "my");
    strcpy(messages[2], "dear");
    strcpy(messages[3], "friends");

    while (!shut.load()) {
        int size = recv(fd, buf, 15, 0);
        if (size < 0) {
            close(fd);
            lg << strerror(errno) << std::endl;
            return;
        }
        buf[14] = '\0';
        lg << size << std::endl;
        lg << buf << std::endl;
        send(fd, messages[rand() % 4], 30, 0);
    }

    send(fd, "shutdown", sizeof ("shutdown"), 0);
    close(fd);
}

int main() {
    /*int fd = 0;

    sockaddr_in addr;


    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        std::cerr << strerror(errno) << "\n";
        return -1;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(30000);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd, (sockaddr *) &addr, sizeof(addr)) < 0) {
        std::cerr << strerror(errno) << "\n";
        return -2;
    }
    listen(fd, 5);

    sockaddr_in client_addr;
    socklen_t sin_size=sizeof(struct sockaddr_in);
    int client_fd = accept(fd, (sockaddr *) &client_addr, &sin_size);
    if (client_fd < 0) {
        std::cerr << strerror(errno) << "\n";
        return -3;
    }

    std::thread communication_thread(communicate, client_fd);
    std::string control_str;

    do {
        std::cin >> control_str;
    } while (control_str != "shutdown");

    shut.store(true);
    communication_thread.join();

    close(fd);
    lg.close();*/





    Server server;

    std::string str;

    while (true) {
        getline(std::cin, str);

        if (str == "start")
            server.start(8080, 10, 900, 200, 2000);
        else if (str == "shutdown")
            server.shutdown();
        else
            break;
    }


    return 0;
}

