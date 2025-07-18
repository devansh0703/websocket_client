#include "cli_handler.hpp"
#include "websocket_client.hpp"
#include "websocket_client_plain.hpp"
#include "message_handler.hpp"
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <memory>

int main(int argc, char* argv[]) {
    try {
        // Parse command line arguments
        websocket_client::CLIHandler cli;
        if (!cli.parse(argc, argv)) {
            return 1;
        }

        // Create IO context
        boost::asio::io_context ioc;

        // Create message handler
        websocket_client::MessageHandler msg_handler;

        // Create WebSocket client based on security flag
        if (cli.isSecure()) {
            // Secure connection
            boost::asio::ssl::context ssl_ctx{boost::asio::ssl::context::tlsv12_client};

            // Configure SSL context
            ssl_ctx.set_verify_mode(boost::asio::ssl::verify_peer);
            ssl_ctx.set_default_verify_paths();

            auto client = std::make_shared<websocket_client::WebSocketClient>(ioc, ssl_ctx);

            client->connect(
                cli.getHost(),
                cli.getPort(),
                cli.getTarget(),
                [&msg_handler](const std::string& msg) {
                    msg_handler.handleMessage(msg);
                },
                [](const std::string& error) {
                    std::cerr << "Error: " << error << std::endl;
                }
            );

            // Run IO context in a separate thread
            std::thread ioc_thread([&ioc]() {
                ioc.run();
            });

            // Main input loop
            std::string line;
            while (std::getline(std::cin, line)) {
                if (line == "quit" || line == "exit") {
                    break;
                }
                
                client->send(msg_handler.formatMessage(line));
            }

            // Clean shutdown
            client->close();
            ioc.stop();
            ioc_thread.join();
        } else {
            // Non-secure connection
            auto client = std::make_shared<websocket_client::WebSocketClientPlain>(ioc);

            client->connect(
                cli.getHost(),
                cli.getPort(),
                cli.getTarget(),
                [&msg_handler](const std::string& msg) {
                    msg_handler.handleMessage(msg);
                },
                [](const std::string& error) {
                    std::cerr << "Error: " << error << std::endl;
                }
            );

            // Run IO context in a separate thread
            std::thread ioc_thread([&ioc]() {
                ioc.run();
            });

            // Main input loop
            std::string line;
            while (std::getline(std::cin, line)) {
                if (line == "quit" || line == "exit") {
                    break;
                }
                
                client->send(msg_handler.formatMessage(line));
            }

            // Clean shutdown
            client->close();
            ioc.stop();
            ioc_thread.join();
        }

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
