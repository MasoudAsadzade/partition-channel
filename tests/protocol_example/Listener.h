#pragma once
#include <PTPLib/Channel.hpp>
#include <PTPLib/Header.hpp>
#include <PTPLib/PartitionConstant.hpp>


#include <thread>
#include <unistd.h>
#include <chrono>


#include "SMTSolver.cc"

class Listener {
    Channel channel;
    SMTSolver * solver;
    PTPLib::synced_stream & stream;
    PTPLib::StoppableWatch timer;
    int nCommands;
    int instanceNum;
    static std::string resultToString(SMTSolver::Result res);

public:
    Listener(PTPLib::synced_stream & ss) : stream(ss) {}

    void setNCommand(int inc, int nc) { instanceNum = inc; nCommands = nc; }

    void solver_worker();

    PTPLib::Task readChannelQuery();

    void execute(PTPLib::Task task);

    std::vector<std::thread> vecOfThreads;

    ~Listener() { }

    void interrupt(const std::string& command);
    void handleMessage(std::pair<PTPLib::safe_ptr<PTPLib::net::Header>, std::string> & header, int randNumber);
    void handleStop();
    void worker(PTPLib::Threads tname, int seed, const std::string & td_min, const std::string & td_max);
    void start_thread(PTPLib::Threads tname, int seed = 0, const std::string & td_min = std::string(),
                      const std::string & td_max = std::string());
    std::pair<PTPLib::safe_ptr<PTPLib::net::Header>, std::string> readCommand(int counter);

    void pull_clause_worker(int seed, const std::string & n_min, const std::string & n_max);
    void push_clause_worker(int seed, const std::string & n_min, const std::string & n_max);
    bool read_lemma(std::vector<std::pair<std::string, int>>  &lemmas, PTPLib::safe_ptr< PTPLib::net::Header> &header);
    void write_lemma(std::map<std::string, std::vector<std::pair<std::string, int>>> const & lemmas, PTPLib::safe_ptr< PTPLib::net::Header> & header);

    Channel & getChannel() { return channel;};

};
