//
// Author:
//

#pragma once
#include <exception>
#include <iostream>


class Exception : public std::exception {
private:
    const std::string msg;

public:
    explicit Exception(const std::string &message) :
            msg(message) {}

    explicit Exception(const char *file, unsigned line, const std::string &message) :
            msg(message + " at " + file + ":" + std::to_string(line)) {}

    const char *what() const throw() { return this->msg.c_str(); }
};

