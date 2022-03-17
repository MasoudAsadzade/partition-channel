#include <iostream>
#include <string>
#include "Listener.h"
#include "SMTSolver.h"


std::string Listener::resultToString(SMTSolver::Result res) {
    if (res == SMTSolver::Result::UNKNOWN)
        return "unknown";
    else if (res == SMTSolver::Result::SAT)
        return "sat";
    else if (res == SMTSolver::Result::UNSAT)
        return "unsat";
    return "undefined";
}

void Listener::runSolver() {
    while (not channel.shouldTerminate())
    {
        solver->search();
        std::unique_lock<std::mutex> lk(channel.getMutex());
        if (solver->getResult() != SMTSolver::Result::UNKNOWN) {
            channel.waitQueryOrTermination(lk);
            if (channel.shouldTerminate())
                break;
        }

        assert(channel.shouldStop());
        if (channel.shouldInjectClause())
        {
            stream.println(true ? PTPLib::Color::FG_Cyan : PTPLib::Color::FG_DEFAULT,
                           "[t SOLVER ] inject clause ");
            for ( const auto &clauses : solver->m_pulledClauses ) {
                std::this_thread::sleep_for(std::chrono::milliseconds (100));
            }
        }
        if (not channel.isEmpty_query())
        {
            PTPLib::Task task = readChannelQuery();
            stream.println(PTPLib::Color::FG_Green, "[t SOLVER ] -> ", task.command,"ing...");
            execute(task);
        }

    }
    stream.println(PTPLib::Color::FG_Green, "[t SOLVER ] -> ", resultToString(solver->getResult()));

}

void doParition(const std::string & node) { }

PTPLib::Task Listener::readChannelQuery()
{
    std::pair<PTPLib::net::Header, std::string> f_query = channel.get_FrontQuery();
    if (f_query.first["command"] == PTPLib::Command.Incremental)
        return PTPLib::Task {
                .command = PTPLib::Task::incremental,
                .smtlib = f_query.second
        };
    else if (f_query.first["command"] == PTPLib::Command.Partition)
        doParition(f_query.first["node"]);

    return PTPLib::Task{
            .command = PTPLib::Task::resume
    };
}

void Listener::execute(PTPLib::Task task)
{
    channel.pop_front_query();
    if (channel.isEmpty_query())
        channel.clearShouldStop();
    else channel.setShouldStop();
}


void Listener::interrupt(const std::string& command) {
    stream.println(PTPLib::Color::FG_Yellow, "[t LISTENER ] Solver Should  -> " + command);
    if (command == PTPLib::Command.Stop) {
        channel.setTerminate();
        channel.setShouldStop();
    }
    else if (command == PTPLib::Command.ClauseInjection)
    {
        channel.setInjectClause();
        channel.setShouldStop();
    }
    else
        channel.setShouldStop();
}

void Listener::handleMessage(std::pair<PTPLib::net::Header, std::string> & header, int randNumber)
{
    std::string seed = "1000";
    stream.println(PTPLib::Color::FG_Red, "[t LISTENER -> ",header.first["command"],
                   " command is received! ]" );

    if (header.first["command"] == PTPLib::Command.Solve)
    {
        solver = std::make_unique<SMTSolver>(channel, header.first, stream, timer);
        start_Thread( PTPLib::Threads::SOLVER, randNumber);
    }
    else if (header.first["command"] == PTPLib::Command.Stop)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds (100));
        handleStop();
    }
    else if (header.first["command"] == PTPLib::Command.Lemmas) {

        start_Thread( PTPLib::Threads::CLAUSEPULL, randNumber, seed, seed);
        start_Thread( PTPLib::Threads::CLAUSEPUSH, randNumber, seed, seed);
    }
    else if (header.first["command"] == PTPLib::Command.Terminate) {
        exit(0);
    }
    else {
        std::unique_lock<std::mutex> lk(channel.getMutex());
//        channel.move_Header(header.first);
        channel.setShouldStop();
        channel.push_back_query(header);
        lk.unlock();
        channel.notify_all();

    }
}

void Listener::handleStop()
{
    if (not this->solver)
        return;
    interrupt(PTPLib::Command.Stop);
    channel.notify_all();
    for (std::thread& th : vecOfThreads) {
        if (th.joinable())
            th.join();
    }
    std::scoped_lock<std::mutex> lk(channel.getMutex());
    channel.resetChannel();
}

void Listener::worker(PTPLib::Threads tname, int seed, const std::string & td_min, const std::string & td_max) {

    switch (tname) {
        case PTPLib::Threads::SOLVER:
            runSolver();
            break;

        case PTPLib::Threads::CLAUSEPUSH:
            solver->clausePush(seed, td_min, td_max);
            break;

        case PTPLib::Threads::CLAUSEPULL:
            solver->clausePull(seed, td_min, td_max);
            break;

        case PTPLib::Threads::MEMORYCHECK:
//                do_task();
            break;
    }

}

void Listener::start_Thread(PTPLib::Threads tname, int seed , const std::string & td_min, const std::string & td_max )
{
    std::thread th(&Listener::worker, this, tname, seed, td_min, td_max);
    vecOfThreads.push_back( std::move(th) );
}

std::pair<PTPLib::net::Header, std::string> Listener::readCommand(int counter)
{
    std::string payload;
    int rDuration = 0;
    if (counter == 1)
        rDuration = 5000;
    else {
        std::srand(counter + timer.elapsed_time_microseconds());
        rDuration = 1 + (std::rand() % (1000 - 1 + 1));
    }
//    stream.println( PTPLib::Color::FG_Yellow, "************ waiting time *********", rDuration);
    std::this_thread::sleep_for(std::chrono::milliseconds (rDuration));

    PTPLib::net::Header header;
    header["name"] = "instance.smt2";
    if (counter == 0) {
        header["command"] = PTPLib::Command.Solve;
        header["name"] = "instance.smt2";
        payload = "solve( " + header["name"] + " )";
    }

    else if (counter == 1) {
        header["command"] = PTPLib::Command.Lemmas;
    }

    else if (counter == nCommands) {
        header["name"] = "instance.smt2";
        header["command"] = PTPLib::Command.Stop;
    }

    else if (counter % 2 == 0) {
        header["command"] = PTPLib::Command.Incremental;
        header["name"] = "instance_" + to_string(counter) + ".smt2";
        payload = "move ( " + header["name"] + " )";
    }

    else {
        header["command"] = PTPLib::Command.Partition;
        header["node"] = "[" + to_string(counter) + "]";
    }

    return std::make_pair(header, payload);
}