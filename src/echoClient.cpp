//
// Created by liad on 12/25/18.
//
#include <mutex>
#include <thread>
#include <iostream>
#include <stdlib.h>
#include "../include/connectionHandler.h"
#include "Task.cpp"
/**
* This code assumes that the server replies the exact text the client sent it (as opposed to the practical session example)
*/
int main (int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " host port" << std::endl << std::endl;
        return -1;
    }
    std::string host = argv[1];
    short port = atoi(argv[2]);

   // ConnectionHandler connectionHandler(host, port);
   // if (!connectionHandler.connect()) {
   //     std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
   //     return 1;
   // }
   // std::mutex mutex;
   // Task task1(connectionHandler, mutex);
   // Task task2(connectionHandler, mutex);
//
   // std::thread th1(&Task::sendToServer, &task1);
   // std::thread th2(&Task::getFromServer, &task2);
   // th1.join();
   // th2.join();
    return 0;
}
