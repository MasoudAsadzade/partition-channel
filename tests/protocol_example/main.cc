#include <iostream>
#include <string>
#include "SMTSolver.h"
#include "Listener.cc"
#include <PTPLib/PartitionConstant.hpp>


int main(int argc, char** argv) {

    if (argc == 1) {
        std::cout << "Usage: " << argv[0] << " <seed>" << std::endl;
        return 1;
    }


    PTPLib::synced_stream stream (std::clog);
    PTPLib::StoppableWatch solving_timeout_watch;

    std::srand(atoi(argv[1]));
    int number_instances =  1 + (std::rand() % 5);
    stream.println( PTPLib::Color::FG_Yellow,
                    "------------------- Total Number of Instances: ",number_instances," ------------------- ");

//Number of commands for each instance

    Listener listener(stream);
    int instanceNum = 1;
    while (number_instances != 0) {
        solving_timeout_watch.start();

        std::srand(static_cast<std::uint_fast8_t>(solving_timeout_watch.elapsed_time_microseconds()));
        int nCommands = 3 + (std::rand() % (10 - 3 + 1));
        listener.setNCommand(instanceNum, nCommands);
        stream.println( PTPLib::Color::FG_Yellow,
                        "********************************** Instance: ", instanceNum, " **********************************");
        stream.println( PTPLib::Color::FG_Yellow,
                        "( Number of Commands: ", nCommands, ")");
        instanceNum++;
        int command_counter = 0;

        number_instances--;

        while (solving_timeout_watch.elapsed_time_second() < static_cast<std::uint_fast8_t>(60)) {

            auto header_payload = listener.readCommand(command_counter);
            stream.println(PTPLib::Color::FG_Red, "[t LISTENER ] -> ", header_payload.first->at("command"),
                           " is received!" );
            assert(not header_payload.first->at("command").empty());
            if (header_payload.first->at("command") == PTPLib::Command.Stop) {
                break;
            }
            listener.handleMessage(header_payload, std::rand());
            command_counter++;



        }
        solving_timeout_watch.reset();
        listener.handleStop();
    }
    return 0;
}


