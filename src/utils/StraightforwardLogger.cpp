//
// Created by dszhdankin on 16.01.2021.
//

#include "StraightforwardLogger.h"

StraightforwardLogger::StraightforwardLogger(const std::string &filename): _log_mutex(), _file_out(filename),
_out(_file_out) {

}

StraightforwardLogger::StraightforwardLogger(std::ostream &stream): _log_mutex(), _file_out(), _out(stream) {

}

void StraightforwardLogger::saveLog(const std::string &log) {
    std::lock_guard<std::mutex> lock(_log_mutex);
    _out << log << std::endl;
}