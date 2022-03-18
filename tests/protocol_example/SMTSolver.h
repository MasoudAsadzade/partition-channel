#pragma once
#include <PTPLib/Channel.hpp>
#include <PTPLib/Printer.hpp>
#include <PTPLib/Header.hpp>
#include <chrono>


class SMTSolver {
public:
    enum class Result { SAT, UNSAT, UNKNOWN };
private:
    Channel & channel;
    PTPLib::net::Header & header;

    bool learnSomeClauses(std::vector<std::pair<std::string ,int>> & learned_clauses);
    Result doSolve();
    Result result;

public:

    PTPLib::synced_stream & stream;
    PTPLib::StoppableWatch & timer;


    SMTSolver(Channel & ch, PTPLib::net::Header & hd, PTPLib::synced_stream & st, PTPLib::StoppableWatch & tm) :
         channel (ch),
         header  (hd),
         stream  (st),
         timer   (tm),
         result (Result::UNKNOWN) {}

    Result getResult() const    { return result; }
    Channel& getChannel()       { return channel; }

    void search();
    void inject_clauses(std::vector<std::pair<std::string, int>> & clauses);

};
