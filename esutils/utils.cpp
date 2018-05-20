#include <iostream>
#include <vector>

#include <arpa/inet.h>

#include "utils.hpp"
#include "exceptions.hpp"

namespace util
{

BaseConfig::BaseConfig() : m_endpoint{ AF_INET, 0, { /*in_addr_t*/ 0 } },
    m_socket{ AF_INET, SOCK_STREAM, IPPROTO_TCP },
    m_max_msg_size{ MSG_MAX_SIZE },
    m_use_tcp{ true },
    m_use_verbose_output{ false }
{
    // empty
    std::cout << "BaseConfig::BaseConfig() was invoked\n"; // dbg
}
} // namespace util
