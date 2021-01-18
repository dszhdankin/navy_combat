//
// Created by dszhdankin on 13.01.2021.
//

#ifndef NAVY_COMBAT_WRITER_H
#define NAVY_COMBAT_WRITER_H
#include "IWriter.h"

class Writer: public IWriter{
private:
    int _fd;

public:
    Writer(int fd);

    bool writeTokens(const std::string &token) override;

    virtual ~Writer();
};


#endif //NAVY_COMBAT_WRITER_H
