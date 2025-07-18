#pragma once

#include <string>
#include <functional>
#include <CLI/CLI.hpp>

namespace websocket_client {

class CLIHandler {
public:
    CLIHandler();

    bool parse(int argc, char* argv[]);
    std::string getHost() const { return host_; }
    std::string getPort() const { return port_; }
    std::string getTarget() const { return target_; }
    bool isSecure() const { return secure_; }

private:
    CLI::App app_{"WebSocket Client"};
    std::string host_{"echo.websocket.org"};
    std::string port_{"443"};
    std::string target_{"/"};
    bool secure_{true};
    bool no_secure_flag_{false};
};

} // namespace websocket_client
