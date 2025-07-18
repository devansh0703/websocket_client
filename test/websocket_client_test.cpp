#include <gtest/gtest.h>
#include "websocket_client.hpp"
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <thread>
#include <chrono>

namespace websocket_client {
namespace test {

class WebSocketClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        ssl_ctx_.set_verify_mode(boost::asio::ssl::verify_peer);
        ssl_ctx_.set_default_verify_paths();
    }

    boost::asio::io_context ioc_;
    boost::asio::ssl::context ssl_ctx_{boost::asio::ssl::context::tlsv12_client};
};

TEST_F(WebSocketClientTest, CreateClient) {
    auto client = std::make_shared<WebSocketClient>(ioc_, ssl_ctx_);
    ASSERT_NE(client, nullptr);
}

TEST_F(WebSocketClientTest, ConnectToEchoServer) {
    auto client = std::make_shared<WebSocketClient>(ioc_, ssl_ctx_);
    bool received = false;

    client->connect(
        "echo.websocket.org",
        "443",
        "/",
        [&received](const std::string& msg) {
            // Just check that we received a message, as the echo server might modify it
            EXPECT_FALSE(msg.empty());
            received = true;
        },
        [](const std::string& error) {
            FAIL() << "Error: " << error;
        }
    );

    // Run the io_context in a separate thread
    std::thread ioc_thread([&]() {
        ioc_.run();
    });

    // Give it some time to connect and set up
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Send test message
    client->send("test message");

    // Wait for response with timeout
    auto start = std::chrono::steady_clock::now();
    while (!received && 
           std::chrono::duration_cast<std::chrono::seconds>(
               std::chrono::steady_clock::now() - start).count() < 5) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Clean up
    client->close();
    ioc_.stop();

    // Wait for thread to finish
    if (ioc_thread.joinable()) {
        ioc_thread.join();
    }

    EXPECT_TRUE(received) << "Did not receive response from echo server";

    EXPECT_TRUE(received);
}

} // namespace test
} // namespace websocket_client
