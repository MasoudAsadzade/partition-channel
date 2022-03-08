/*
 * Copyright (c) 2022, Antti Hyvarinen <antti.hyvarinen@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <iostream>

namespace PartitionChannel
{
    static struct {
        const std::string Partition = "partition";
        const std::string Stop = "stop";
        const std::string ClauseInjection = "inject";
        const std::string Incremental = "incremental";
        const std::string CnfClauses = "cnf-clauses";
        const std::string Cnflearnts = "cnf-learnts";
        const std::string Solve = "solve";
        const std::string Lemmas = "lemmas";
        const std::string Terminate = "terminate";
    } Command;

    struct Task {
        const enum {
            incremental, resume, partition, stop
        } command;

        std::string smtlib;
    };

    enum Threads {
        Communication, ClausePull, ClausePush, MemCheck
    };

    enum Status {
        unknown, sat, unsat
    };

    static bool s_colorMode;
}

