//
// Created by dszhdankin on 13.01.2021.
//

#ifndef NAVY_COMBAT_IWRITER_H
#define NAVY_COMBAT_IWRITER_H
#include <string>

class IWriter {
public:
    virtual bool writeTokens(const std::string &token) = 0;
    virtual ~IWriter() {};
};

#endif //NAVY_COMBAT_IWRITER_H
