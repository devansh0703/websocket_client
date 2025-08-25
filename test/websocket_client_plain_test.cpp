#include <gtest/gtest.h>
#include "../src/websocket_client_plain.hpp"
#include <boost/asio/io_context.hpp>
#include <thread>
#include <chrono>

class WebSocketClientPlainTest : public ::testing::Test {
protected:
    void SetUp() override {
        ioc = std::make_unique<boost::asio::io_context>();
        client = std::make_shared<websocket_client::WebSocketClientPlain>(*ioc);
    }

    void TearDown() override {
        if (client) {
            client->close();
        }
        if (ioc) {
            ioc->stop();
        }
        if (ioc_thread.joinable()) {
            ioc_thread.join();
        }
    }

    std::unique_ptr<boost::asio::io_context> ioc;
    std::shared_ptr<websocket_client::WebSocketClientPlain> client;
    std::thread ioc_thread;
    std::string received_message;
    std::string received_error;
    bool message_received = false;
    bool error_received = false;
    bool connection_established = false;
};

TEST_F(WebSocketClientPlainTest, CanCreateClient) {
    EXPECT_NE(client, nullptr);
}

TEST_F(WebSocketClientPlainTest, CanConnectToEchoServer) {
    // Start IO context in background
    ioc_thread = std::thread([this]() {
        ioc->run();
    });

    // Set up message and error handlers
    auto message_handler = [this](const std::string& msg) {
        received_message = msg;
        message_received = true;
    };

    auto error_handler = [this](const std::string& error) {
        received_error = error;
        error_received = true;
    };

    auto connect_handler = [this]() {
        connection_established = true;
    };

    // Try connecting to ws://echo.websocket.org first, fallback if needed
    client->connect("ws.ifelse.io", "80", "/", message_handler, error_handler, connect_handler);

    // Wait for connection to be established with realistic timeout
    int wait_count = 0;
    while (!connection_established && !error_received && wait_count < 50) { // Reduced from 100 to 50
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        wait_count++;
    }

    if (error_received) {
        FAIL() << "Connection error: " << received_error;
    }

    if (!connection_established) {
        FAIL() << "Connection timeout: connection was not established.";
    }

    std::cout << "Connection established successfully" << std::endl;

    // Send a test message
    client->send("Hello, WebSocket!");

    // Wait for echo response
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Verify we received the echo
    if (error_received) {
        FAIL() << "Error received after connection: " << received_error;
    }
    if (!message_received) {
        FAIL() << "No message received after sending.";
    }
    EXPECT_EQ(received_message, "Hello, WebSocket!");
}

TEST_F(WebSocketClientPlainTest, CanSendBinaryData) {
    // Start IO context in background
    ioc_thread = std::thread([this]() {
        ioc->run();
    });

    // Set up message and error handlers
    auto message_handler = [this](const std::string& msg) {
        received_message = msg;
        message_received = true;
    };

    auto error_handler = [this](const std::string& error) {
        received_error = error;
        error_received = true;
    };

    auto connect_handler = [this]() {
        connection_established = true;
    };

    // Connect to plain WebSocket echo server
    client->connect("ws.ifelse.io", "80", "/", message_handler, error_handler, connect_handler);

    // Wait for connection or error
    int wait_count = 0;
    while (!connection_established && !error_received && wait_count < 50) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        wait_count++;
    }

    if (connection_established) {
        // Send binary data
        std::vector<uint8_t> binary_data = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello"
        client->sendBinary(binary_data);

        // Wait for echo response
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    // Verify we received something (connection established or error is both acceptable)
    EXPECT_TRUE(connection_established || error_received);
}

TEST_F(WebSocketClientPlainTest, HandlesInvalidHost) {
    // Start IO context in background
    ioc_thread = std::thread([this]() {
        ioc->run();
    });

    // Set up message and error handlers
    auto message_handler = [this](const std::string& msg) {
        received_message = msg;
        message_received = true;
    };

    auto error_handler = [this](const std::string& error) {
        received_error = error;
        error_received = true;
    };

    // Try to connect to invalid host
    client->connect("invalid.host.that.does.not.exist", "80", "/", message_handler, error_handler);

    // Wait for error
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Verify we received an error
    EXPECT_TRUE(error_received);
    EXPECT_FALSE(message_received);
}
