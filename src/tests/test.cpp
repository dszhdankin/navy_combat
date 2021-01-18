//TODO get familiar with gtest
#include <iostream>
#include <poll.h>
#include <memory>
#include <string>
#include "../utils/IReader.h"
#include "../utils/Reader.h"
#include "../utils/IWriter.h"
#include "../utils/Writer.h"

class B {
public:
    B() { std::cout << "created" << std::endl; }
    ~B() { std::cout << "destroyed" << std::endl; }
};

struct A {
    B b[5][5];
};

int main() {
    std::shared_ptr<IReader> reader(new Reader(0, 30));
    std::shared_ptr<IWriter> writer(new Writer(1));
    pollfd pfd[1];
    pfd[0].fd = 0;
    pfd[0].events = POLLIN;

    std::string curTok;
    while (true) {
        poll(pfd, 1, 600000);
        curTok = reader->getToken();
        if (curTok == "end")
            break;
        else
            std::cout << curTok << std::endl;
    }
    std::cout << "wtf" << std::endl;
    while (true) {
        poll(pfd, 1, 600000);
        curTok = reader->getToken();
        if (curTok == "end")
            break;
        writer->writeTokens(curTok);
    }
    A a;
    return 0;
}

