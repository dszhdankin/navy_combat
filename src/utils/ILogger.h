//
// Created by dszhdankin on 16.01.2021.
//

#ifndef NAVY_COMBAT_ILOGGER_H
#define NAVY_COMBAT_ILOGGER_H
#include <string>

class ILogger {
public:
    virtual void saveLog(const std::string &log) = 0;
    virtual ~ILogger() {};
};


#endif //NAVY_COMBAT_ILOGGER_H
