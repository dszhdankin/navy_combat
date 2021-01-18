//
// Created by dszhdankin on 12.01.2021.
//

#ifndef NAVY_COMBAT_READER_H
#define NAVY_COMBAT_READER_H
#include "IReader.h"


class Reader: public IReader {
private:
    int _fd = 0, _buf_size = 0, _data_in_buf_size = 0, _next_sym_pos = 0;
    char *_buf = nullptr;

    bool hasNext();
    char next();

public:
    Reader(int fd, int bufSize);

    int getBufSize() { return _buf_size; }
    void setBufSize(int bufSize);

    std::string getToken() override;

    virtual ~Reader();

};


#endif //NAVY_COMBAT_READER_H
