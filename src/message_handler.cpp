#include "message_handler.hpp"
#include <iostream>

namespace websocket_client {

MessageHandler::MessageHandler()
    : message_callback_([](const std::string& msg) {
        std::cout << "Received: " << msg << std::endl;
    })
{
}

void MessageHandler::handleMessage(const std::string& message) {
    if (message_callback_) {
        message_callback_(message);
    }
}

std::string MessageHandler::formatMessage(const std::string& message) const {
    return message;
}

void MessageHandler::setMessageCallback(std::function<void(const std::string&)> callback) {
    message_callback_ = std::move(callback);
}

} // namespace websocket_client
