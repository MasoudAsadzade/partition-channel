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
    std::srand(atoi(argv[1]));
    int nInstances =  1 + (std::rand() % (5 - 1 + 1));
    stream.println( PTPLib::Color::FG_Yellow,
                    "------------------- Number of Instances: ",nInstances," ------------------- ");
    PTPLib::StoppableWatch watch;

    int nCommands = 3 + (std::rand() % (10 - 3 + 1));
    std::srand(atoi(argv[1]) + time(NULL));
    Listener listener(stream);
    int temp = 0;
    while (nInstances != 0) {
        temp++;
        watch.start();
        int counter = 0;
        listener.setNCommand(nCommands);
        nInstances--;
        stream.println( PTPLib::Color::FG_Yellow,
                        "____________________ Instance: ", temp, "_____________________________");
        while (watch.elapsed_time_second() < static_cast<std::uint_fast8_t>(60)) {

            auto header = listener.readCommand(counter);
            assert(not header.first["command"].empty());
            listener.handleMessage(header, std::rand());
            counter++;

            if (header.first["command"] == PTPLib::Command.Stop) {
                listener.handleStop();
                break;
            }

        }
        watch.reset();
        listener.handleStop();
    }
    return 0;
}


