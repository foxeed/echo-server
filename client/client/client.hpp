#pragma once

#include <vector>
#include <string_view>
#include "../../esutils/utils.hpp"

namespace client
{

constexpr std::string_view CLIENT_HELP_TEXT{
    R"raw(Command line arguments:
    -h or --help        this page
    -v or --verbose     log everything to stdout
    -a <addr>           the default address for listening
    -p <port>           the default port for listening
    -proto <tcp/udp>    which protocol the server will use; default: tcp
    -m <message>        message, max 64 Kb
)raw" };

struct Config : public util::BaseConfig
{
    void InitConfig(int argc, char *argv[]) override;
    [[noreturn]] inline void SendHelp() const override;
    inline void Log(std::string const& str, char prefix = 'c') const override;

    ~Config() override = default;

    std::vector<char> m_msg;
};

class Client
{
public:
    Client(Config const& cfg);
    ~Client() = default;

    void Start();

private:
    sockaddr_in SetEndpointFromCfg();

    Config m_cfg;
    sockaddr_in m_endpoint;
    socklen_t m_endpoint_size;  // for convenience, since we have m_endpoint already
    std::vector<char> m_buff;
};
} // namespace client
