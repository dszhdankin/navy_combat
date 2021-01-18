//
// Created by dszhdankin on 13.01.2021.
//

#include "Writer.h"
#include <unistd.h>
#include <fcntl.h>
#include <cassert>

Writer::Writer(int fd) {
    assert(fd >= 0);
    _fd = fd;
    //Ensure socket is nonblocking
    int flags = fcntl(_fd, F_GETFL, 0);
    fcntl(_fd, F_SETFL, flags | O_NONBLOCK);
}

Writer::~Writer() noexcept {}

//With ;
bool Writer::writeTokens(const std::string &token) {
    int res = write(_fd, token.c_str(), token.size());
    if (res != token.size())
        return false;
    return true;
}