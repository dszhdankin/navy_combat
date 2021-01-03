#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <cassert>

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
        serv_addr.sin_port = port;
        serv_addr.sin_family = AF_INET;

        if (connect(fd, (sockaddr *) &serv_addr, sizeof (serv_addr)) < 0) {
            std::cout << strerror(errno) << "\n";
            throw std::exception();
        }

        connected = true;
    }

    void send_message(const char *buf, int size) {
        char recv_buf[30];
        memset(recv_buf, 0, sizeof (recv_buf));

        if (send(fd, buf, size, 0) < 0) {
            throw std::exception();
        }

        int recv_size = recv(fd, recv_buf, 30, 0);
        if (recv_size < 0) {
            throw std::exception();
        }

        std::cout << recv_size << "\n"
        << recv_buf << "\n";
    }
};

int main() {
    std::string message;
    MessageSender sender;
    char addr[] = "127.0.0.1";

    sender.connect_to_host(addr, 5000);

    do {
        std::getline(std::cin, message);
        sender.send_message(message.c_str(), message.size() + 1);
    } while (message != "stop");
}
