#pragma once

#include <iosfwd>
#include <vector>
#include <thread>

#include <netinet/in.h>

#include "../../esutils/utils.hpp"

namespace srv
{
struct Config : public util::BaseConfig
{
    Config() : m_total_threads{ 1 } {}
    void InitConfig(int argc, char *argv[]) override;
    [[noreturn]] inline void SendHelp() const override;
    inline void Log(std::string const& str, char prefix = 'd') const override;
    ~Config() override {}

    std::size_t m_total_threads;
};

/**
 * \brief The Dispatcher spawns worker threads when needed
 */
class Dispatcher
{
public:
    Dispatcher(Config const& config);
    ~Dispatcher() = default;

    void Start();

private:
    void InitListenSocket(int& server_listen_socket);
    sockaddr_in SetEndpointFromCfg();
    void Dispatch();

    Config m_cfg;
    // TODO: move m_end* to threads
    sockaddr_in m_endpoint;
    socklen_t m_endpoint_size;  // for convenience, since we have m_endpoint already
//    std::vector<std::thread> m_threads;
};

} // namespace srv
