#pragma once

#include <stdexcept>
#include <string>

namespace util
{

class EchoServerException
        : public std::runtime_error
{
public:
    EchoServerException(std::string const& err_text)
        : std::runtime_error(err_text)
    {}

    using std::runtime_error::what;
    char const* what() const noexcept override;
};

class ConfigException
        : public EchoServerException
{
public:
    ConfigException(std::string const& err_text)
        : EchoServerException(err_text)
    {}

    using EchoServerException::what;
    char const* what() const noexcept override;
};

[[noreturn]] inline void exit_with_fault()
{
    exit(EXIT_FAILURE);
}

[[noreturn]] inline void exit_gracefully()
{
    exit(EXIT_SUCCESS);
}
} // namespace util
