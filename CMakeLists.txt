cmake_minimum_required(VERSION 3.12.3)
project(partition_channel)

set(CMAKE_CXX_STANDARD 17)

add_library(partition_channel PartitionChannelLibrary.h src/Channel.h  src/Header.h src/Exception.h
        src/lib.h src/ChannelConfig.h src/Printer.h src/Timer.h)