#pragma once

#include <string>
#include <functional>

namespace websocket_client {

class MessageHandler {
public:
    MessageHandler();

    // Handle incoming messages from the server
    void handleMessage(const std::string& message);

    // Format outgoing messages
    std::string formatMessage(const std::string& message) const;

    // Set callback for when messages are received
    void setMessageCallback(std::function<void(const std::string&)> callback);

private:
    std::function<void(const std::string&)> message_callback_;
};

} // namespace websocket_client
