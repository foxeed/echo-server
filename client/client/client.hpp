#pragma once

#include <vector>
#include "../../esutils/utils.hpp"

namespace client
{

struct Config : public util::BaseConfig
{
    Config() {}
    void InitConfig(int argc, char *argv[]) override;
    [[noreturn]] inline void SendHelp() const override;
    inline void Log(std::string const& str, char prefix = 'c') const override;
    ~Config() override {}

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
