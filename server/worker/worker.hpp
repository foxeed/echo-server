#pragma once

#include <iosfwd>

namespace wrk
{

class Worker
{
public:
    Worker();
    ~Worker();

private:
    int handle;
};

} //namespace wrk
