#include <iostream>
#include <string>
#include "SMTSolver.h"
#include <stdlib.h>
#include <random>
#include <mutex>
#include <thread>


bool SMTSolver::learnSomeClauses(std::vector<std::pair<std::string ,int>> & learned_clauses) {

    if (std::rand() % 3 == 0)
        return false;
    for (int i = 0; i < std::rand() % 10000 ; ++i) {
        learned_clauses.emplace_back(std::make_pair("assert()",i % 10));
    }
    return not learned_clauses.empty();
}

SMTSolver::Result SMTSolver::doSolve() {
    std::vector<std::pair<std::string ,int>>  toPublishClauses;
    if (not learnSomeClauses(toPublishClauses)) {
        return std::rand() < RAND_MAX / 2 ? Result::SAT : Result::UNSAT;
    }

    {
        std::scoped_lock<std::mutex> lk(channel.getMutex());
        stream.println(PTPLib::Color::FG_Green, "[t SOLVER -> add learned clauses to channel buffer, Size=",
                       toPublishClauses.size() );
        channel.insert_learned_clause(toPublishClauses);
    }
    return std::rand() < RAND_MAX / 100 ? Result::UNSAT : Result::SAT;
}

void SMTSolver::search() {
    result = Result::UNKNOWN;
    while (result == Result::UNKNOWN and not channel.shouldStop())
    {
        result = doSolve();

        std::this_thread::sleep_for(std::chrono::milliseconds (500));
    }
}

void SMTSolver::inject_clauses(std::vector<std::pair<std::string, int>> & clauses)
{
    std::this_thread::sleep_for(std::chrono::milliseconds (100));
    clauses.clear();
}