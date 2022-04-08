/*
 * Copyright (c) Matteo Marescotti <Matteo.marescotti@usi.ch>
 * Copyright (c) 2022, Antti Hyvarinen <antti.hyvarinen@gmail.com>
 * Copyright (c) 2022, Seyedmasoud Asadzadeh <seyedmasoud.asadzadeh@usi.ch>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef PTPLIB_NET_SMTSEVENT_H
#define PTPLIB_NET_SMTSEVENT_H

#include "PTPLib/net/Header.hpp"

#include <vector>
#include <sstream>


namespace PTPLib::net {
    struct SMTS_Event {
        PTPLib::net::Header header;
        std::string body;
        SMTS_Event(PTPLib::net::Header && hd, std::string && str) {
            this->header = std::move(hd);
            this->body = std::move(str);
        }
    };
}

#endif //PTPLIB_NET_SMTSEVENT_H