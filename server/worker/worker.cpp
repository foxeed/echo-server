#include <iostream>
#include <string>

#include <sys/types.h>
#include <unistd.h>

#include "worker.hpp"

namespace wrk
{

Worker::Worker() : handle(0)
{
    std::cout << "[w] Worker thread " << handle << " started\n";
}


Worker::~Worker()
{
    std::cout << "[w] Worker thread " << handle << " died\n";
}

} // namespace wrk
