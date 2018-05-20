#include <iostream>
#include <vector>
#include <cstring>
#include <utility>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../../esutils/exceptions.hpp"
#include "client.hpp"

namespace client
{
constexpr std::string_view CLIENT_HELP_TEXT{
    R"raw(Command line arguments:
    -h or --help        this page
    -v or --verbose     log everything to stdout
    -a <addr>           the default address for listening
    -p <port>           the default port for listening
    -proto <tcp/udp>    which protocol the server will use; default: tcp
    -m <message>        message, max 64 Kb)raw"
};

Client::Client(Config const& cfg) :
    m_cfg{ cfg },
    m_buff{ std::move(cfg.m_msg) }
{
    m_endpoint = SetEndpointFromCfg();
    m_endpoint_size = sizeof m_endpoint;

    m_cfg.Log("client created");
}

// FIXME: this is very buggy
sockaddr_in Client::SetEndpointFromCfg()
{
    sockaddr_in endpoint;
    endpoint.sin_family = m_cfg.m_endpoint.m_family;
    endpoint.sin_port = htons(m_cfg.m_endpoint.m_port);
    endpoint.sin_addr = m_cfg.m_endpoint.m_addr;
    memset(&endpoint.sin_zero, 0, sizeof(endpoint.sin_zero));
    return endpoint;
}

void Client::Start()
{
    int connect_socket =0;
//    int connect_socket = socket(m_cfg.m_socket.m_family,
//                                m_cfg.m_socket.m_socket_t,
//                                m_cfg.m_socket.m_proto_flavour);

//    if(-1 == getsockname(connect_socket,
//                         reinterpret_cast<sockaddr *>(&m_endpoint),
//                         &m_endpoint_size))
//    {
//        close(connect_socket);
//        throw util::EchoServerException("getsockname() failed");
//    }
//    else
//    {
//        m_cfg.Log("started at address: " + std::string(inet_ntoa(m_endpoint.sin_addr))
//            + ", port: " + std::to_string(ntohs(m_endpoint.sin_port))
//            + ", using protocol: " + (m_cfg.m_use_tcp ? "TCP" : "UDP"));
//    }

    sockaddr_in endpoint;
    endpoint.sin_family = m_cfg.m_endpoint.m_family;
    endpoint.sin_port = htons(m_cfg.m_endpoint.m_port);
    endpoint.sin_addr = m_cfg.m_endpoint.m_addr;
    memset(&endpoint.sin_zero, 0, sizeof(endpoint.sin_zero));

    socklen_t size_endpoint = sizeof endpoint;

    if(m_cfg.m_use_tcp)
    {
        connect_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(connect_socket == -1)
        {
            throw util::EchoServerException("cannot create socket");
        }

        m_cfg.Log("started, connect_socket: " + std::to_string(connect_socket));

        if(-1 == getsockname(connect_socket,
                             reinterpret_cast<sockaddr *>(&endpoint),
                             &size_endpoint))
        {
            close(connect_socket);
            throw util::EchoServerException("getsockname() failed");
        }
        else
        {
            m_cfg.Log("started at address: " + std::string(inet_ntoa(m_endpoint.sin_addr))
                + ", port: " + std::to_string(ntohs(m_endpoint.sin_port))
                + ", using protocol: " + (m_cfg.m_use_tcp ? "TCP" : "UDP"));
        }

        if (-1 == connect(connect_socket,
                          reinterpret_cast<sockaddr *>(&m_endpoint),
                          sizeof m_endpoint))
        {
            close(connect_socket);
            throw util::EchoServerException("connect() failed");
        }

        if (send(connect_socket, &m_buff[0], m_buff.size(), 0) < 0)
        {
            close(connect_socket);
            throw util::EchoServerException("cannot send packet");
        }

        auto bytes = static_cast<std::size_t>(recv(connect_socket,
                                                   &m_buff[0],
                                                   m_buff.capacity(),
                                                   0));
        if (bytes > 0 )
        {
            if(bytes <= m_cfg.m_max_msg_size
               && m_buff[bytes] != '\0')
            {
                m_buff[bytes] = '\0';
            }
        }
        else
        {
            close(connect_socket);
            throw util::EchoServerException("cannot receive packet");
        }

        m_cfg.Log("received packet, got " + std::to_string(bytes) + " bytes:\n\""
            + std::string(m_buff.data(), bytes) + "\"");

        m_cfg.Log("shutting down socket: " + std::to_string(connect_socket));
        shutdown(connect_socket, SHUT_RDWR);
    }
    else    // use UDP
    {
        connect_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        ssize_t bytes_sent = sendto(connect_socket,
                                    reinterpret_cast<void *>(&m_buff[0]),
                                    m_buff.capacity(),
                                    0,
                                    reinterpret_cast<sockaddr *>(&endpoint),
                                    size_endpoint);
        if (bytes_sent < 0)
        {
            close(connect_socket);
            throw util::EchoServerException("cannot send packet");
        }

        auto bytes = static_cast<std::size_t>(recvfrom(connect_socket,
                                              reinterpret_cast<void *>(&m_buff[0]),
                                              m_buff.capacity(),
                                              0,
                                              reinterpret_cast<sockaddr *>(&endpoint),
                                              &size_endpoint));
        if (bytes > 0 )
        {
            if(bytes <= m_cfg.m_max_msg_size &&
                    m_buff[bytes] != '\0')
            {
                m_buff[bytes] = '\0';
            }
        }
        else
        {
            close(connect_socket);
            throw util::EchoServerException("cannot receive packet");
        }

        m_cfg.Log("received packet, got " + std::to_string(bytes) + " bytes:\n\""
            + std::string(m_buff.data(), bytes) + "\"");
    }

    m_cfg.Log("closed connection at socket: " + std::to_string(connect_socket));
    close(connect_socket);
}

void Config::InitConfig(int argc, char *argv[])
{
    std::vector<std::string> args(argv, argv+argc);
    for(std::size_t i = 0; i < args.size(); ++i)
    {
        if (args[i] == "-h" || args[i] == "--help")
            SendHelp(); // this will end the program execution
        if (args[i] == "-v" || args[i] == "--verbose")
            m_use_verbose_output = true;
        if (args[i] == "-m" && i + 1 < args.size())
        {
            m_msg = std::vector<char>(args[i + 1].begin(), args[i + 1].end());
        }
        if (args[i] == "-a" && i + 1 < args.size())
        {
            // do not allow to start at 0.0.0.0
//            m_endpoint.m_addr.s_addr = inet_addr(args[i + 1].c_str());
            inet_pton(AF_INET, args[i + 1].c_str(), &m_endpoint.m_addr);
//            if(inet_aton(args[i + 1].c_str(), &m_endpoint.m_addr) == 0)
//                throw util::ConfigException("invalid address");
        }
        if (args[i] == "-p" && i + 1 < args.size())
        {
            uint16_t temp;
            try
            {
                temp = static_cast<uint16_t>(std::stoi(args[i + 1]));
            }
            catch(std::invalid_argument const&)
            {
                throw util::ConfigException("invalid port");
            }
            catch(std::out_of_range const&)
            {
                throw util::ConfigException("invalid port");
            }
            m_endpoint.m_port = temp;
        }
        if (args[i] == "-proto" && i + 1 < args.size())
        {
            std::string const& proto = args[i + 1];
            if(proto.size() > 3) // more letters than in "tcp" or "udp"
                throw util::ConfigException("unsupported protocol");

            // cumbersome, but portable way of safely lowercasing characters:
            std::locale loc;
            std::string lower_cased(proto.size(), 0);
            for(std::size_t ch = 0; ch < proto.size(); ++ch)
                lower_cased[ch] = std::tolower(proto[ch], loc);

            if(lower_cased == "tcp")
            {
                m_socket.m_socket_t = SOCK_STREAM;
                m_socket.m_proto_flavour = IPPROTO_TCP;
            }
            else if(lower_cased == "udp")
            {
                m_use_tcp = false;
                m_socket.m_socket_t = SOCK_DGRAM;
                m_socket.m_proto_flavour = IPPROTO_UDP;
            }
            else
                throw util::ConfigException("unsupported protocol");
        }
    }

}

void Config::SendHelp() const noexcept
{
    std::cout << CLIENT_HELP_TEXT;
    util::exit_gracefully();
}

void Config::Log(std::string const& str, char prefix) const
{
    if(m_use_verbose_output)
        std::cout << "[" << prefix << "] " << str << "\n";
}

} // namespace client
