#include <iostream>
#include "dispatcher/dispatcher.hpp"
#include "../esutils/exceptions.hpp"

int main(int argc, char *argv[])
{
    srv::Config cfg;
    try
    {
        cfg.InitConfig(argc, argv);
    }
    catch (util::ConfigException const& e)
    {
        std::cerr << "Wrong argument: " << e.what();
        util::exit_with_fault();
    }

    srv::Dispatcher dsp(cfg);
    try
    {
        dsp.Start();
    }
    catch(util::EchoServerException const& e)
    {
        std::cerr << "Runtime exception: " << e.what();
        util::exit_with_fault();
    }
    catch(...)
    {
        std::cerr << "Server crashed unexpectedly";
        util::exit_with_fault();
    }

    return 0;
}
