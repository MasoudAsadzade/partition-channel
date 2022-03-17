#pragma once
#include <PTPLib/Channel.hpp>
#include <PTPLib/Header.hpp>
#include <PTPLib/PartitionConstant.hpp>
#include <thread>
#include "unistd.h"
#include "SMTSolver.cc"
#include <chrono>

class Listener {
    Channel channel;
    std::unique_ptr<SMTSolver> solver;
    static std::string resultToString(SMTSolver::Result res);
    PTPLib::synced_stream & stream;
    PTPLib::StoppableWatch timer;
    int nCommands;

public:
    Listener(int nc, PTPLib::synced_stream & ss) : stream(ss), nCommands(nc) {}
    void runSolver();
    PTPLib::Task readChannelQuery();
    void execute(PTPLib::Task task);
    std::vector<std::thread> vecOfThreads;

    ~Listener() { }

    void interrupt(const std::string& command);
    void handleMessage(std::pair<PTPLib::net::Header, std::string> & header, int randNumber);
    void handleStop();
    void worker(PTPLib::Threads tname, int seed, const std::string & td_min, const std::string & td_max);
    void start_Thread(PTPLib::Threads tname, int seed = 0, const std::string & td_min = std::string(),
                      const std::string & td_max = std::string());
    std::pair<PTPLib::net::Header, std::string> readCommand(int counter);
};
