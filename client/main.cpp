#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>

#include "../esutils/exceptions.hpp"
#include "client/client.hpp"

int main(int argc, char *argv[])
{
    client::Config cfg;
    try
    {
        cfg.InitConfig(argc, argv);
    }
    catch (util::ConfigException const& e)
    {
        std::cerr << "Wrong argument: " << e.what();
        util::exit_with_fault();
    }

    client::Client client(cfg);
    try
    {
        client.Start();
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
}
