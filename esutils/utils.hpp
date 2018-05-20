#pragma once

#include <iosfwd>
#include <string>
#include <netinet/in.h>

namespace util
{
struct BaseConfig
{
    BaseConfig();
    virtual void InitConfig(int argc, char *argv[]) = 0;
    [[noreturn]] virtual  inline void SendHelp() const = 0;
    virtual inline void Log(std::string const& str, char prefix) const = 0;
    virtual ~BaseConfig() {}

    struct ListenSocket
    {
        int m_family;
        int m_socket_t;
        int m_proto_flavour;
    };
    struct Endpoint
    {
        sa_family_t m_family;
        uint16_t m_port;
        in_addr m_addr;
    };

    Endpoint m_endpoint;
    ListenSocket m_socket;
    static const ssize_t m_max_msg_size = 64 * 1024;
    bool m_use_tcp;
    bool m_use_verbose_output;
    char __pad[2];
};
} // namespace util
