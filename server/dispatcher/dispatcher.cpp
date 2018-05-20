#include <iostream>
#include <string>
#include <string_view>
#include <locale>
#include <vector>
#include <thread>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "dispatcher.hpp"
#include "../worker/worker.hpp"
#include "../../esutils/exceptions.hpp"

namespace srv
{

Dispatcher::Dispatcher(Config const& cfg) : m_cfg{ cfg }
{
    m_endpoint = SetEndpointFromCfg();
    m_endpoint_size = sizeof m_endpoint;

    m_cfg.Log("dispatcher created with " + std::to_string(m_cfg.m_total_threads) + " threads");
}

void Dispatcher::InitListenSocket(int& server_listen_socket)
{
    server_listen_socket = socket(m_cfg.m_socket.m_family,
                                  m_cfg.m_socket.m_socket_t,
                                  m_cfg.m_socket.m_proto_flavour);
    if(server_listen_socket == -1)
    {
        throw util::EchoServerException("cannot create socket");
    }

    m_cfg.Log("started, server_listen_socket: " + std::to_string(server_listen_socket));

    if(-1 == bind(server_listen_socket,
                  reinterpret_cast<sockaddr *>(&m_endpoint),
                  sizeof m_endpoint)) // TODO: investigate
    {
        close(server_listen_socket);
        throw util::EchoServerException("bind() failed");
    }

    m_cfg.Log("bound to all addresses");

    if(!m_cfg.m_use_tcp)
        return;

    if(-1 == listen(server_listen_socket, 10))
    {
        close(server_listen_socket);
        throw util::EchoServerException("listen() failed");
    }

    m_cfg.Log("listening to all addresses");
}

sockaddr_in Dispatcher::SetEndpointFromCfg()
{
    sockaddr_in endpoint;
    endpoint.sin_family = m_cfg.m_endpoint.m_family;
    endpoint.sin_port = htons(m_cfg.m_endpoint.m_port);
    endpoint.sin_addr.s_addr = m_cfg.m_endpoint.m_addr.s_addr;
    memset(&endpoint.sin_zero, 0, sizeof(endpoint.sin_zero));
    return endpoint;
}

void Dispatcher::Start()
{
    int server_listen_socket = 0;
    InitListenSocket(server_listen_socket);

    if(-1 == getsockname(server_listen_socket,
                         reinterpret_cast<sockaddr *>(&m_endpoint),
                         &m_endpoint_size))
    {
        close(server_listen_socket);
        throw util::EchoServerException("getsockname() failed");
    }
    else
    {
        m_cfg.Log("started at address: " + std::string(inet_ntoa(m_endpoint.sin_addr))
            + ", port: " + std::to_string(ntohs(m_endpoint.sin_port))
            + ", using protocol: " + (m_cfg.m_use_tcp ? "TCP" : "UDP"));
    }

    std::vector<char> buff(1024, 0);    // FIXME: compute magic constants at compile-time
    // TODO: make while(current_size <= max_msg_size) {m_ostream.write(buff); current_size+=bytes;}

    if(m_cfg.m_use_tcp)
    {
        // TODO: make it receive first message with the first four bytes noting
        // how many bytes will be sent afterwards -- at least some warranty!
        // in pseudocode:
        // receive_bytes(socket, length, buffer) { //... }
        // int len = 0;
        // receive_bytes(fd, sizeof(len) /*4 on x86_64*/, (void*)(&len));
        // receive_bytes(fd, len, (void*)buffer);
        while(true)
        {
            struct sockaddr_in peer_addr;
            socklen_t peer_addr_size = sizeof(peer_addr);

            int connect_socket = accept(server_listen_socket,
                                        reinterpret_cast<sockaddr *>(&peer_addr),
                                        &peer_addr_size);
            if (connect_socket < 0)
            {
              close(server_listen_socket);
              throw util::EchoServerException("accept() failed");
            }

            m_cfg.Log("accepted connection at socket: " + std::to_string(connect_socket));

            // FIXME: receives only 1024 bytes
            auto bytes = static_cast<std::size_t>(recv(connect_socket,
                                                       &buff[0],
                                                       buff.capacity(),
                                                       0));
            if (bytes > 0 )
            {
                if(bytes <= m_cfg.m_max_msg_size
                   && buff[bytes] != '\0')
                {
                    buff[bytes] = '\0';
                }
            }
            else
            {
                close(server_listen_socket);
                throw util::EchoServerException("cannot receive packet");
            }

            m_cfg.Log("accepted packet, got " + std::to_string(bytes) + " bytes:\n\""
                + std::string(buff.data(), bytes) + "\"");

            if (send(connect_socket, &buff[0], buff.capacity(), 0) < 0)
            {
                close(server_listen_socket);
                throw util::EchoServerException("cannot send packet");
            }

            if (shutdown(connect_socket, SHUT_RDWR) == -1)
            {
                close(connect_socket);
                close(server_listen_socket);
                throw util::EchoServerException("shutdown() failed");
            }

            m_cfg.Log("closed connection at socket: " + std::to_string(connect_socket));
            close(connect_socket);

            std::string_view buff_view(buff.data(), buff.size());
            auto pos = buff_view.find(DBG_STOP_TOKEN);
            if(pos != std::string_view::npos
               && buff_view[pos + DBG_STOP_TOKEN.size()] == '\0')
            {
                m_cfg.Log("got a stop phrase, shutting down");
                break;
            }

        }

    }
    else    // use UDP
    {
        while(true)
        {
            auto bytes = static_cast<std::size_t>(recvfrom(server_listen_socket,
                                                           reinterpret_cast<void *>(&buff[0]),
                                                           buff.capacity(),
                                                           0,
                                                           reinterpret_cast<sockaddr *>(&m_endpoint),
                                                           &m_endpoint_size));
            if (bytes > 0 )
            {
                if(bytes <= m_cfg.m_max_msg_size &&
                        buff[bytes] != '\0')
                {
                    buff[bytes] = '\0';
                }
            }
            else
            {
                close(server_listen_socket);
                throw util::EchoServerException("cannot receive packet");
            }

            m_cfg.Log("received packet, got " + std::to_string(bytes) + " bytes:\n\""
                + std::string(buff.data(), bytes) + "\"");

            auto bytes_sent = static_cast<std::size_t>(sendto(server_listen_socket,
                                                              reinterpret_cast<void *>(&buff[0]),
                                                              buff.capacity(),
                                                              0,
                                                              reinterpret_cast<sockaddr *>(&m_endpoint),
                                                              m_endpoint_size));
            if (bytes_sent == 0)
            {
                close(server_listen_socket);
                throw util::EchoServerException("cannot send packet");
            }

            // TODO: test against finite state machine, which matches the stop phrase
            std::string_view buff_view(buff.data(), buff.size());
            auto pos = buff_view.find(DBG_STOP_TOKEN);
            if(pos != std::string_view::npos
               && buff_view[pos + DBG_STOP_TOKEN.size()] == '\0')
            {
                m_cfg.Log("got a stop phrase, shutting down");
                break;
            }
        }
    }

    close(server_listen_socket);
    m_cfg.Log("stopped normally");

//    for(std::size_t id{ 0 }; id < m_total_threads; ++id)
//    {
//        std::thread thread([id]()
//            {
//                Worker wrk;
//            });

//        m_threads.push_back(std::move(thread));
//    }

//    std::cout << "[d] all threads are started\n";

//    for(auto& thread : m_threads)
//    {
//        thread.join();
//        m_cur_threads++;
//        std::cout << "[d] joined " << m_cur_threads << " threads, " << m_total_threads - m_cur_threads << " left\n";
    //    }
}

void Config::InitConfig(int argc, char *argv[])
{
    std::vector<std::string> args(argv, argv+argc);
    for(std::size_t i = 0; i < args.size(); ++i)
    {
        if (args[i] == "-h" || args[i] == "--help")
        {
            SendHelp(); // this will end the program execution
        }
        if (args[i] == "-v" || args[i] == "--verbose")
        {
            m_use_verbose_output = true;
        }
        if (args[i] == "-a" && i + 1 < args.size())
        {
            // do not allow to start at 0.0.0.0
            if(inet_aton(args[i + 1].c_str(), &m_endpoint.m_addr) == 0)
                throw util::ConfigException("invalid address");
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
            {
                throw util::ConfigException("unsupported protocol");
            }
        }
    }

}

void Config::Log(std::string const& str, char prefix) const
{
    if(m_use_verbose_output)
        std::cout << "[" << prefix << "] " << str << "\n";
}

void Config::SendHelp() const noexcept
{
    std::cout << SERVER_HELP_TEXT;
    util::exit_gracefully();
}
} // namespace srv
