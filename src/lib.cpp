/*
 * Copyright (c) 2022, Antti Hyvarinen <antti.hyvarinen@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "lib.h"

template<>
const std::string to_string<bool>(const bool & b) {
    return b ? "true" : "false";
}

std::istream & split(std::istream & stream,
                     const char delimiter,
                     std::function<void(const std::string &)> callback) {
    std::string sub;
    while (std::getline(stream, sub, delimiter)) {
        callback(sub);
    }
    return stream;
}


std::istream & split(std::istream & stream,
                     const char delimiter,
                     std::vector<std::string> & vector) {
    return split(stream, delimiter, [&vector](const std::string & sub) {
        vector.push_back(sub);
    });
}


std::vector<std::string> split(const std::string & string, const std::string & delimiter, uint32_t limit) {
    std::vector<std::string> vector;
    split(string, delimiter, [&vector](const std::string & sub) {
        vector.push_back(sub);
    }, limit);
    return vector;
}

void split(const std::string & string,
           const std::string & delimiter,
           std::function<void(const std::string &)> callback,
           uint32_t limit) {
    size_t b = 0;
    size_t e;
    while (true) {
        if (limit != 0 && --limit == 0)
            e = std::string::npos;
        else
            e = string.find(delimiter, b);
        callback(string.substr(b, e - b));
        if (e == std::string::npos)
            return;
        b = e + delimiter.size();
    }
}

std::string & replace(std::string & string, const std::string & from, const std::string & to, size_t n) {
    if (from.empty())
        return string;
    size_t start_pos = 0;
    while ((start_pos = string.find(from, start_pos)) != std::string::npos) {
        string.replace(start_pos, from.length(), to);
        start_pos += to.length();
        if (n > 0) {
            if (n == 1)
                break;
            n--;
        }
    }
    return string;
}

std::string & operator%(std::string & string, const std::pair<std::string, std::string> & pair) {
    return ::replace(string, pair.first, pair.second);
}
