#include "src/cli_handler.hpp"
#include <iostream>

int main() {
    websocket_client::CLIHandler cli;
    const char* argv[] = {
        "program",
        "--host", "localhost",
        "--port", "8080",
        "--target", "/echo",
        "--no-secure"
    };
    
    std::cout << "Before parsing - isSecure: " << cli.isSecure() << std::endl;
    
    bool result = cli.parse(8, const_cast<char**>(argv));
    std::cout << "Parse result: " << result << std::endl;
    std::cout << "After parsing - isSecure: " << cli.isSecure() << std::endl;
    std::cout << "Host: " << cli.getHost() << std::endl;
    std::cout << "Port: " << cli.getPort() << std::endl;
    std::cout << "Target: " << cli.getTarget() << std::endl;
    
    return 0;
}
