#pragma once
#include <PTPLib/Channel.hpp>
#include <PTPLib/Header.hpp>
#include <PTPLib/PartitionConstant.hpp>
#include <PTPLib/ThreadPool.hpp>

#include <unistd.h>
#include <chrono>

#include "SMTSolver.cc"

class Communicator {
    PTPLib::Channel & channel;
    SMTSolver solver;
    PTPLib::synced_stream & stream;
    PTPLib::StoppableWatch timer;
    bool color_enabled;
    PTPLib::ThreadPool & th_pool;
    std::future<SMTSolver::Result> future;
    std::atomic<std::thread::id> thread_id;

public:

    Communicator(PTPLib::Channel & ch, PTPLib::synced_stream & ss, const bool & ce, double seed, PTPLib::ThreadPool & th)
        :
        channel(ch),
        solver(ch, ss, timer, ce, seed),
        stream(ss),
        color_enabled(ce),
        th_pool(th)
     {}

    bool execute_event(const PTPLib::smts_event & event, bool & shouldUpdateSolverAddress);

    bool setStop(PTPLib::smts_event & event);

    void communicate_worker();

    PTPLib::Channel & getChannel() { return channel;};

};
