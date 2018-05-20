#pragma once

#include <iosfwd>
#include <string>
#include <netinet/in.h>

namespace util
{

constexpr ssize_t MSG_MAX_SIZE{ 64 * 1024 };

struct BaseConfig
{
    BaseConfig();
    virtual ~BaseConfig() = default;

    virtual void InitConfig(int argc, char *argv[]) = 0;
    [[noreturn]] virtual  inline void SendHelp() const noexcept = 0;
    virtual inline void Log(std::string const& str, char prefix) const = 0;

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
    char __pad[4];
    std::size_t const m_max_msg_size;
    bool m_use_tcp;
    bool m_use_verbose_output;
    char ___pad[6];
};
} // namespace util
