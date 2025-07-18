#include "cli_handler.hpp"

namespace websocket_client {

CLIHandler::CLIHandler() {
    app_.add_option("-H,--host", host_, "WebSocket server hostname")
        ->default_val("echo.websocket.org");
    
    app_.add_option("-p,--port", port_, "WebSocket server port")
        ->default_val("443");
    
    app_.add_option("-t,--target", target_, "WebSocket target path")
        ->default_val("/");
    
    // Add the no-secure flag as a member variable
    app_.add_flag("--no-secure", no_secure_flag_, "Disable secure WebSocket (use ws:// instead of wss://)");
}

bool CLIHandler::parse(int argc, char* argv[]) {
    try {
        app_.parse(argc, argv);
        
        // Check if no-secure flag was set
        if (no_secure_flag_) {
            secure_ = false;
            // Change default port to 80 for non-secure connections if still using default
            if (port_ == "443") {
                port_ = "80";
            }
        }
        
        return true;
    } catch (const CLI::ParseError &e) {
        app_.exit(e);
        return false;
    }
}

} // namespace websocket_client
