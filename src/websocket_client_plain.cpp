#include "websocket_client_plain.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <iostream>

namespace websocket_client {

WebSocketClientPlain::WebSocketClientPlain(boost::asio::io_context& ioc)
    : resolver_(boost::asio::make_strand(ioc))
    , ws_(boost::asio::make_strand(ioc))
    , connected_(false) {
}

void WebSocketClientPlain::connect(
    const std::string& host,
    const std::string& port,
    const std::string& target,
    MessageHandler onMessage,
    ErrorHandler onError,
    ConnectHandler onConnect) {
    
    host_ = host;
    target_ = target;
    onMessage_ = onMessage;
    onError_ = onError;
    onConnect_ = onConnect;

    // Look up the domain name
    resolver_.async_resolve(
        host,
        port,
        boost::beast::bind_front_handler(
            &WebSocketClientPlain::onResolve,
            shared_from_this()
        )
    );
}

void WebSocketClientPlain::onResolve(
    boost::beast::error_code ec,
    boost::asio::ip::tcp::resolver::results_type results) {
    
    if (ec) {
        return fail(ec, "resolve");
    }

    // Set the timeout for the operation
    boost::beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    boost::beast::get_lowest_layer(ws_).async_connect(
        results,
        boost::beast::bind_front_handler(
            &WebSocketClientPlain::onConnect,
            shared_from_this()
        )
    );
}

void WebSocketClientPlain::onConnect(
    boost::beast::error_code ec,
    boost::asio::ip::tcp::resolver::results_type::endpoint_type ep) {
    
    if (ec) {
        return fail(ec, "connect");
    }

    // Turn off the timeout on the tcp_stream, because
    // the websocket stream has its own timeout system.
    boost::beast::get_lowest_layer(ws_).expires_never();

    // Set suggested timeout settings for the websocket
    ws_.set_option(
        boost::beast::websocket::stream_base::timeout::suggested(
            boost::beast::role_type::client
        )
    );

    // Set a decorator to change the User-Agent of the handshake
    ws_.set_option(boost::beast::websocket::stream_base::decorator(
        [](boost::beast::websocket::request_type& req) {
            req.set(boost::beast::http::field::user_agent,
                std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-cpp");
        }
    ));

    // Create the Host header value
    std::string host_header = host_;
    // Only add port if it's not the default port for the protocol
    if (ep.port() != 80 && ep.port() != 443) {
        host_header += ':' + std::to_string(ep.port());
    }

    // Perform the websocket handshake
    ws_.async_handshake(
        host_header,
        target_,
        boost::beast::bind_front_handler(
            &WebSocketClientPlain::onHandshake,
            shared_from_this()
        )
    );
}

void WebSocketClientPlain::onHandshake(boost::beast::error_code ec) {
    if (ec) {
        return fail(ec, "handshake");
    }

    connected_ = true;
    
    // Notify that connection is established
    if (onConnect_) {
        onConnect_();
    }
    
    // Read a message
    doRead();
}

void WebSocketClientPlain::doRead() {
    // Read a message into our buffer
    ws_.async_read(
        buffer_,
        boost::beast::bind_front_handler(
            &WebSocketClientPlain::onRead,
            shared_from_this()
        )
    );
}

void WebSocketClientPlain::onRead(
    boost::beast::error_code ec,
    std::size_t bytes_transferred) {
    
    boost::ignore_unused(bytes_transferred);

    if (ec) {
        return fail(ec, "read");
    }

    // Process the message
    if (onMessage_) {
        std::string message = boost::beast::buffers_to_string(buffer_.data());
        onMessage_(message);
    }

    // Clear the buffer
    buffer_.consume(buffer_.size());

    // Read another message
    doRead();
}

void WebSocketClientPlain::send(const std::string& message) {
    if (!connected_) {
        if (onError_) {
            onError_("Not connected");
        }
        return;
    }

    // Send the message
    ws_.async_write(
        boost::asio::buffer(message),
        boost::beast::bind_front_handler(
            &WebSocketClientPlain::onWrite,
            shared_from_this()
        )
    );
}

void WebSocketClientPlain::sendBinary(const std::vector<uint8_t>& data) {
    if (!connected_) {
        if (onError_) {
            onError_("Not connected");
        }
        return;
    }

    // Set binary mode
    ws_.binary(true);

    // Send the binary data
    ws_.async_write(
        boost::asio::buffer(data),
        boost::beast::bind_front_handler(
            &WebSocketClientPlain::onWrite,
            shared_from_this()
        )
    );
}

void WebSocketClientPlain::onWrite(
    boost::beast::error_code ec,
    std::size_t bytes_transferred) {
    
    boost::ignore_unused(bytes_transferred);

    if (ec) {
        return fail(ec, "write");
    }

    // Reset to text mode after binary send
    ws_.binary(false);
}

void WebSocketClientPlain::close() {
    if (!connected_) {
        return;
    }

    connected_ = false;

    // Close the WebSocket connection
    ws_.async_close(
        boost::beast::websocket::close_code::normal,
        boost::beast::bind_front_handler(
            &WebSocketClientPlain::onClose,
            shared_from_this()
        )
    );
}

void WebSocketClientPlain::onClose(boost::beast::error_code ec) {
    if (ec) {
        return fail(ec, "close");
    }

    // The WebSocket is closed
    connected_ = false;
}

void WebSocketClientPlain::fail(boost::beast::error_code ec, const char* what) {
    connected_ = false;
    if (onError_) {
        onError_(std::string(what) + ": " + ec.message());
    }
}

} // namespace websocket_client
