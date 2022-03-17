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
        channel.insert(toPublishClauses);
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


void SMTSolver::clausePull(int seed, const  std::string & n1, const std::string & n2)
{
    timer.start();
    int pullDuration = atoi(n1.c_str()) + ( seed % ( atoi(n2.c_str())- atoi(n1.c_str()) + 1 ) );
    std::chrono::duration<double> wakeupAt = std::chrono::milliseconds (pullDuration);
    while (true) {
        if (getChannel().shouldTerminate()) break;
        std::unique_lock<std::mutex> lk(getChannel().getMutex());
//            PTPLib::PrintStopWatch printer("[t pull wait for pull red]", stream,PTPLib::Color::Code::FG_Red);
        if (not getChannel().waitFor(lk, wakeupAt))
        {
            if (getChannel().shouldTerminate()) break;
            PTPLib::net::Header header;
            lk.unlock();
            std::vector<std::pair<std::string, int>> lemmas;
            if (this->lemma_pull(lemmas, header))
            {
                lk.lock();
                stream.println(PTPLib::Color::FG_Blue, "[t PULL -> pulled clauses to buffer, Size=",lemmas.size() );
                m_pulledClauses[header["node"]].insert(std::end(m_pulledClauses[header["node"]]),
                                                         std::begin(lemmas), std::end(lemmas));

                channel.setInjectClause();
                channel.setShouldStop();
                lk.unlock();
            }
        }
        else
            break;
    }
}

void SMTSolver::clausePush(int seed, const  std::string & n1,  const std::string & n2)
{
    int pushDuration = atoi(n1.c_str()) + ( seed % ( atoi(n2.c_str()) - atoi(n1.c_str()) + 1 ) );
    stream.println(PTPLib::Color::FG_Yellow, "[t PUSH -> timout : ", pushDuration," ms");
    std::chrono::duration<double> wakeupAt = std::chrono::milliseconds (pushDuration);
    std::map<std::string, std::vector<std::pair<std::string, int>>> m_clauses;
    while (not getChannel().shouldTerminate()) {
//            PTPLib::PrintStopWatch timer1("[t push wait for push red]", stream, PTPLib::Color::Code::FG_Blue);
        std::unique_lock<std::mutex> lk(getChannel().getMutex());
        if (not getChannel().waitFor(lk, wakeupAt))
        {
            if (getChannel().shouldTerminate())
                break;
            else if (not getChannel().empty())
            {
                m_clauses = getChannel().getClauses();
                stream.println(PTPLib::Color::FG_Yellow, "[t PUSH -> push learned clauses to Cloud Clause");
                getChannel().clear();
                PTPLib::net::Header header = this->header.copy({"name"});
                lk.unlock();

                lemma_push(m_clauses, header);
                m_clauses.clear();
            }
            else std::cout << "[t PUSH] Channel empty! " <<std::endl;

        }
        else
            break;
    }
}


bool SMTSolver::lemma_pull(std::vector<std::pair<std::string, int>>  & lemmas, PTPLib::net::Header & header) {
    std::this_thread::sleep_for(std::chrono::milliseconds (100));
    if (std::rand() % 3 == 0)
        return false;
    for (int i = 0; i < std::rand() % 1000 ; ++i) {
        lemmas.emplace_back(std::make_pair("assert()", i % 10));
    }
    return not lemmas.empty();
}

void SMTSolver::lemma_push(std::map<std::string, std::vector<std::pair<std::string, int>>> const & m_clauses, PTPLib::net::Header & header)
{
    std::this_thread::sleep_for(std::chrono::milliseconds (100));
    for ( const auto &toPushClause : m_clauses )
    {
        stream.println(true ? PTPLib::Color::FG_BrightBlue : PTPLib::Color::FG_DEFAULT,
                       "[t PUSH ] -> push learned clauses to Cloud Clause from Node -> "+
                       header["node"] + " Size: ", toPushClause.second.size());
    }
}