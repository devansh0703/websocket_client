#include <gtest/gtest.h>
#include "message_handler.hpp"

namespace websocket_client {
namespace test {

TEST(MessageHandlerTest, HandleMessage) {
    MessageHandler handler;
    std::string received_message;
    
    handler.setMessageCallback([&received_message](const std::string& msg) {
        received_message = msg;
    });
    
    handler.handleMessage("test message");
    EXPECT_EQ(received_message, "test message");
}

TEST(MessageHandlerTest, FormatMessage) {
    MessageHandler handler;
    std::string formatted = handler.formatMessage("test message");
    EXPECT_EQ(formatted, "test message");
}

} // namespace test
} // namespace websocket_client
