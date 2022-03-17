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
    std::map< std::string, std::vector< std::pair< std::string, int> > >  m_pulledClauses;
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
    void clausePull(int seed, const std::string & n_min, const std::string & n_max);
    void clausePush(int seed, const std::string & n_min, const std::string & n_max);
    bool lemma_pull(std::vector<std::pair<std::string, int>>  &lemmas, PTPLib::net::Header &header);
    void lemma_push(std::map<std::string, std::vector<std::pair<std::string, int>>> const & lemmas, PTPLib::net::Header & header);
};
