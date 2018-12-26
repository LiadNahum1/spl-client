cmake_minimum_required(VERSION 3.12)
project(BGSclient)

set(CMAKE_CXX_STANDARD 14)

add_executable(BGSclient src/connectionHandler.cpp src/echoClient.cpp include/connectionHandler.h src/Task.cpp)