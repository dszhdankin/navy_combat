//
// Created by dszhdankin on 12.01.2021.
//

#ifndef NAVY_COMBAT_IREADER_H
#define NAVY_COMBAT_IREADER_H

#include <string>

class IReader {
public:
    virtual std::string getToken() = 0;
    virtual ~IReader() {};
};

#endif //NAVY_COMBAT_IREADER_H
