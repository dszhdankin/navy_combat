#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <cassert>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <exception>

class MessageSender {
private:
    int fd = -1;
    bool connected = false;

public:
    MessageSender() {
        fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) {
            throw std::exception();
        }
    }

    void connect_to_host(char *host, int port) {
        assert(fd > 0);

        sockaddr_in serv_addr;

        memset(&serv_addr, 0, sizeof (serv_addr));
        serv_addr.sin_addr.s_addr = inet_addr(host);
        serv_addr.sin_port = htons(port);
        serv_addr.sin_family = AF_INET;

        if (connect(fd, (sockaddr *) &serv_addr, sizeof (serv_addr)) < 0) {
            std::cout << strerror(errno) << "\n";
            throw std::exception();
        }

        connected = true;
    }

    void send_message(const char *buf, int size) {


        if (send(fd, buf, size, 0) < 0) {
            throw std::exception();
        }

        /*int recv_size = recv(fd, recv_buf, 4, 0);
        if (recv_size < 0) {
            throw std::exception();
        }*/

        /*std::cout << recv_size << "\n"
        << recv_buf << "\n";*/
    }

    std::string recv_message() {
        char recv_buf[1000];
        memset(recv_buf, 0, sizeof (recv_buf));

        int recv_size = recv(fd, recv_buf, 1000, 0);
        if (recv_size < 0) {
            throw std::exception();
        }

        return std::string(recv_buf);
    }
};

void tryShipsConfiguration(std::vector<std::string> &ships) {
    MessageSender sender;
    char addr[] = "127.0.0.1";

    sender.connect_to_host(addr, 8080);

    std::string dataToSend = "greetings;";

    for (int i = 0; i < ships.size(); i++)
        dataToSend += ships[i];

    sender.send_message(dataToSend.c_str(), dataToSend.size());

    std::string serverResponse = sender.recv_message();

    std::cout << serverResponse << std::endl;
}

int main() {
    /*std::srand(std::time(0));
    std::vector<std::thread> threads;
    int n;
    std::cin >> n;
    for (int i = 0; i < n; i++) {
        std::thread t(dodos_server, 5000, 3 + std::rand() % 4);
        threads.push_back(std::move(t));
    }
    for (int i = 0; i < threads.size(); i++)
        threads[i].join();*/

    std::string inputData;

    while (true) {
        getline(std::cin, inputData);
        if (inputData == "try") {
            std::vector<std::string> ships;
            for (int i = 0; i < 10; i++) {
                std::string curShip;
                getline(std::cin, curShip);
                ships.push_back(curShip);
            }
            tryShipsConfiguration(ships);
        } else {
            break;
        }
    }

    return 0;
}
