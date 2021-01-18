//
// Created by dszhdankin on 12.01.2021.
//

#include "Reader.h"
#include <cstring>
#include <cassert>
#include <unistd.h>
#include <fcntl.h>
#include <cctype>

Reader::Reader(int fd, int bufSize) {
    assert(fd >= 0 && bufSize > 0);

    _buf_size = bufSize;
    _fd = fd;
    _buf = new char[_buf_size];
    //Ensure socket is nonblocking
    int flags = fcntl(_fd, F_GETFL, 0);
    fcntl(_fd, F_SETFL, flags | O_NONBLOCK);

    std::memset(_buf, 0, _buf_size);
}

//WARNING
//Deletes previous buf
//Use carefully
void Reader::setBufSize(int bufSize) {
    assert(bufSize > 0);

    if (_buf)
        delete [] _buf;
    _buf_size = bufSize;
    _buf = new char[_buf_size];

    std::memset(_buf, 0, _buf_size);
}

bool Reader::hasNext() {
    if (_next_sym_pos < _data_in_buf_size)
        return true;

    _data_in_buf_size = 0;
    _next_sym_pos = 0;
    int bytesRead = read(_fd, _buf, _buf_size);
    if (bytesRead > 0)
        _data_in_buf_size = bytesRead;

    return _next_sym_pos < _data_in_buf_size;
}

//Always returns '\0' if there is no next symbol
char Reader::next() {
    if (_next_sym_pos < _data_in_buf_size)
        return _buf[_next_sym_pos++];

    if (hasNext())
        return _buf[_next_sym_pos++];

    return '\0';
}

//Without ;
std::string Reader::getToken() {
    std::string res;
    bool success = false;

    res.reserve(30);
    while (hasNext()) {
        char curSym = next();
        if (curSym == ';') {
            success = true;
            break;
        }
        if (!std::isspace(curSym))
            res.push_back(curSym);
        if (res.size() == 30)
            break;
    }
    if (!success)
        res.clear();

    return res;
}

Reader::~Reader()  {
    if (_buf)
        delete [] _buf;
}
