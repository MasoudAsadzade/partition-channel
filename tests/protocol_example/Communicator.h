#pragma once
#include <PTPLib/Channel.hpp>
#include <PTPLib/Header.hpp>
#include <PTPLib/PartitionConstant.hpp>
#include <PTPLib/ThreadPool.hpp>

#include <unistd.h>
#include <chrono>

#include "SMTSolver.cc"

class Communicator {
    Channel & channel;
    SMTSolver solver;
    PTPLib::synced_stream & stream;
    PTPLib::StoppableWatch timer;
    bool color_enabled;
    PTPLib::ThreadPool th_pool;

public:

    Communicator(Channel & ch, PTPLib::synced_stream & ss, const bool & ce)
    :
        channel(ch),
        stream(ss),
        solver(ch, ss, timer, ce),
        color_enabled(ce),
        th_pool(1) {}

    void solver_worker(const PTPLib::net::Header & header, const std::string & smt_lib);

    PTPLib::Task execute_event(const std::pair<PTPLib::net::Header, std::string> & event);

    void notify_event(std::pair<PTPLib::net::Header, std::string> & header);

    void communicate_worker();

    Channel & getChannel() { return channel;};

};