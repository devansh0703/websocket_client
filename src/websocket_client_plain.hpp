#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace websocket_client {

class WebSocketClientPlain : public std::enable_shared_from_this<WebSocketClientPlain> {
public:
    using MessageHandler = std::function<void(const std::string&)>;
    using ErrorHandler = std::function<void(const std::string&)>;
    using ConnectHandler = std::function<void()>;

    explicit WebSocketClientPlain(boost::asio::io_context& ioc);

    void connect(
        const std::string& host,
        const std::string& port,
        const std::string& target,
        MessageHandler onMessage,
        ErrorHandler onError,
        ConnectHandler onConnect = nullptr
    );

    void send(const std::string& message);
    void sendBinary(const std::vector<uint8_t>& data);
    void close();

private:
    void onResolve(
        boost::beast::error_code ec,
        boost::asio::ip::tcp::resolver::results_type results
    );

    void onConnect(
        boost::beast::error_code ec,
        boost::asio::ip::tcp::resolver::results_type::endpoint_type ep
    );

    void onHandshake(boost::beast::error_code ec);
    void doRead();
    void onRead(boost::beast::error_code ec, std::size_t bytes_transferred);
    void onWrite(boost::beast::error_code ec, std::size_t bytes_transferred);
    void onClose(boost::beast::error_code ec);

    void fail(boost::beast::error_code ec, const char* what);

    boost::asio::ip::tcp::resolver resolver_;
    boost::beast::websocket::stream<boost::beast::tcp_stream> ws_;
    boost::beast::flat_buffer buffer_;
    std::string host_;
    std::string target_;
    MessageHandler onMessage_;
    ErrorHandler onError_;
    ConnectHandler onConnect_;
    bool connected_;
};

} // namespace websocket_client
