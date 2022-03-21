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



void Listener::solver_worker() {
    while (not channel.shouldTerminate())
    {
        solver->search();

        std::unique_lock<std::mutex> lk(channel.getMutex());
        if (solver->getResult() != SMTSolver::Result::UNKNOWN) {
            channel.waitQueryOrTermination(lk);
            if (channel.shouldTerminate())
                break;
        }

//        assert(channel.shouldStop());

        if (channel.shouldInjectClause())
        {
            stream.println(true ? PTPLib::Color::FG_Green : PTPLib::Color::FG_DEFAULT,
                           "[t SOLVER ] inject clause ");
            for ( auto &clauses : channel.get_pulled_clauses() ) {
                if (solver)
                    solver->inject_clauses(clauses.second);
            }
            channel.clear_pulled_clauses();
        }
        if (not getChannel().isEmpty_query())
        {
            PTPLib::Task task = readChannelQuery();
//            stream.println(PTPLib::Color::FG_Green, "[t SOLVER ] -> ", task.command,"ing...");
            execute(task);
        }

    }
    stream.println(PTPLib::Color::FG_Green, "[t SOLVER ] -> ", resultToString(solver->getResult()));

}

void doParition(const std::string & node) { }

PTPLib::Task Listener::readChannelQuery()
{
    std::pair<PTPLib::safe_ptr< PTPLib::net::Header>, std::string> f_query = getChannel().get_FrontQuery();
    if ((*f_query.first)["command"] == PTPLib::Command.Incremental)
        return PTPLib::Task {
                .command = PTPLib::Task::incremental,
                .smtlib = f_query.second
        };
    else if ((*f_query.first)["command"] == PTPLib::Command.Partition)
        doParition((*f_query.first)["node"]);

    return PTPLib::Task{
            .command = PTPLib::Task::resume
    };
}

void Listener::execute(PTPLib::Task task)
{
    getChannel().pop_front_query();
    if (getChannel().isEmpty_query())
        channel.clearShouldStop();
    else channel.setShouldStop();
}


void Listener::interrupt(const std::string& command) {
//    stream.println(PTPLib::Color::FG_Yellow, "[t LISTENER ] Solver Should  -> " + command);
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


void Listener::handleMessage(std::pair<PTPLib::safe_ptr<PTPLib::net::Header>, std::string> & header_payload, int randNumber)
{
    std::string seed = "1000";
//    channel.move_Header(header.first);
    if (header_payload.first->at("command") == PTPLib::Command.Solve)
    {
        solver = new SMTSolver(channel, header_payload.first, stream, timer);
        start_thread( PTPLib::Threads::SOLVER, randNumber);
    }
    else if (header_payload.first->at("command") == PTPLib::Command.Stop)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds (100));
        handleStop();
    }
    else if (header_payload.first->at("command") == PTPLib::Command.Lemmas) {

        start_thread( PTPLib::Threads::CLAUSEPULL, randNumber, seed, seed);
        start_thread( PTPLib::Threads::CLAUSEPUSH, randNumber, seed, seed);
    }
    else if (header_payload.first->at("command") == PTPLib::Command.Terminate) {
        exit(0);
    }
    else {
        std::unique_lock<std::mutex> lk(channel.getMutex());
//        channel.move_Header(header.first);
        channel.setShouldStop();
        getChannel().push_back_query(header_payload);
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
    delete this->solver;
    this->solver = nullptr;
    std::scoped_lock<std::mutex> lk(channel.getMutex());
    channel.resetChannel();
}

void Listener::worker(PTPLib::Threads tname, int seed, const std::string & td_min, const std::string & td_max) {

    switch (tname) {
        case PTPLib::Threads::SOLVER:
            solver_worker();
            break;

        case PTPLib::Threads::CLAUSEPUSH:
            push_clause_worker(seed, td_min, td_max);
            break;

        case PTPLib::Threads::CLAUSEPULL:
            pull_clause_worker(seed, td_min, td_max);
            break;

        case PTPLib::Threads::MEMORYCHECK:
//            do_mem_check();
            break;
    }

}

void Listener::start_thread(PTPLib::Threads tname, int seed , const std::string & td_min, const std::string & td_max )
{
    std::thread th(&Listener::worker, this, tname, seed, td_min, td_max);
    vecOfThreads.push_back( std::move(th) );
}

std::pair<PTPLib::safe_ptr<PTPLib::net::Header>, std::string> Listener::readCommand(int counter)
{
//    struct solver_entry {
//        PTPLib::net::Header header;
//        std::string payload;
//        solver_entry(){}
//    };
    std::string payload;
//    std::pair<std::string, safe_ptr<PTPLib::net::Header> > solver_entry;

    int rDuration = 0;
    if (counter == 1)
        rDuration = 1000;
    else {
        std::srand(counter + timer.elapsed_time_microseconds());
        rDuration = 1 + (std::rand() % (1000 - 1 + 1));
    }
//    stream.println( PTPLib::Color::FG_Yellow, "************ waiting time *********", rDuration);
    std::this_thread::sleep_for(std::chrono::milliseconds (rDuration));

    PTPLib::safe_ptr<PTPLib::net::Header> header;
    header->emplace("name","instance"+ to_string(instanceNum) +".smt2");
    header->emplace("node", "[" + to_string(counter) + "]");
    if (counter == 0) {
        header->emplace("command", PTPLib::Command.Solve);
        payload = "solve( " + header->at("name") + " )";
    }

    else if (counter == 1) {
        header->emplace("command", PTPLib::Command.Lemmas);
    }

    else if (counter == nCommands) {
        header->emplace("command", PTPLib::Command.Stop);
    }

    else if (counter % 2 == 0) {
        header->emplace("command", PTPLib::Command.Incremental);
//        header->emplace("name", "instance_" + to_string(counter) + ".smt2");
        header->emplace("_node", "[" + to_string(counter+1) + "]");
        payload = "move ( " + header->at("name") + " )";
    }

    else {
        header->emplace("command", PTPLib::Command.Partition);
        header->emplace("partitionN", "2");
    }

    return std::make_pair(header, payload);
}

void Listener::push_clause_worker(int seed, const  std::string & n1,  const std::string & n2)
{
    int pushDuration = atoi(n1.c_str()) + ( seed % ( atoi(n2.c_str()) - atoi(n1.c_str()) + 1 ) );
    stream.println(PTPLib::Color::FG_Blue, "[t PUSH -> timout : ", pushDuration," ms");
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
                m_clauses = getChannel().get_learned_clauses();
//                stream.println(PTPLib::Color::FG_Blue, "[t PUSH ] -> push learned clauses to Cloud Clause");
                getChannel().get_learned_clauses();
                PTPLib::safe_ptr<PTPLib::net::Header> header = getChannel().getHeader().copy({"name"});
                lk.unlock();

                write_lemma(m_clauses, header);
                m_clauses.clear();
            }
            else stream.println(PTPLib::Color::FG_Blue, "[t PUSH ] -> Channel empty!");

        }
        else
            break;
    }
}

void Listener::pull_clause_worker(int seed, const  std::string & n1, const std::string & n2)
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
            PTPLib::safe_ptr<PTPLib::net::Header> header;
            lk.unlock();
            std::vector<std::pair<std::string, int>> lemmas;
            if (this->read_lemma(lemmas, header))
            {
                lk.lock();
                stream.println(PTPLib::Color::FG_Black, "[t PULL ] -> pulled clauses to buffer, Size: ",lemmas.size() );

                channel.insert_pulled_clause(lemmas);
                channel.setInjectClause();
                channel.setShouldStop();
                lk.unlock();
            }
        }
        else
            break;
    }
}

bool Listener::read_lemma(std::vector<std::pair<std::string, int>>  & lemmas, PTPLib::safe_ptr<PTPLib::net::Header> & header) {
    std::this_thread::sleep_for(std::chrono::milliseconds (100));
    if (std::rand() % 3 == 0)
        return false;
    for (int i = 0; i < std::rand() % 1000 ; ++i) {
        lemmas.emplace_back(std::make_pair("assert()", i % 10));
    }
    return not lemmas.empty();
}

void Listener::write_lemma(std::map<std::string, std::vector<std::pair<std::string, int>>> const & m_clauses, PTPLib::safe_ptr<PTPLib::net::Header> & header)
{
    std::this_thread::sleep_for(std::chrono::milliseconds (100));
    for ( const auto &toPushClause : m_clauses )
    {
        stream.println(true ? PTPLib::Color::FG_BrightBlue : PTPLib::Color::FG_DEFAULT,
                       "[t PUSH ] -> push learned clauses to Cloud Clause Size: ", toPushClause.second.size());
    }
}