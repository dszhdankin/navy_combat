//
// Created by dszhdankin on 16.01.2021.
//

#ifndef NAVY_COMBAT_STRAIGHTFORWARDLOGGER_H
#define NAVY_COMBAT_STRAIGHTFORWARDLOGGER_H

#include "ILogger.h"
#include <fstream>
#include <mutex>
#include <memory>

class StraightforwardLogger : public ILogger {
private:
    std::ostream &_out;
    std::ofstream _file_out;
    std::mutex _log_mutex;

public:
    StraightforwardLogger(const std::string &filename);
    StraightforwardLogger(std::ostream &stream);
    virtual ~StraightforwardLogger() {}
    void saveLog(const std::string &log) override;
};


#endif //NAVY_COMBAT_STRAIGHTFORWARDLOGGER_H
