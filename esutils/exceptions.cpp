#include "exceptions.hpp"

namespace util
{

// for -Wweak-tables
char const* EchoServerException::what() const noexcept
{
    return runtime_error::what();
}

char const* ConfigException::what() const noexcept
{
    return runtime_error::what();
}
} // namespace util
