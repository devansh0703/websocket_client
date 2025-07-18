#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace websocket_client {

class WebSocketClient : public std::enable_shared_from_this<WebSocketClient> {
public:
    using MessageHandler = std::function<void(const std::string&)>;
    using ErrorHandler = std::function<void(const std::string&)>;

    WebSocketClient(
        boost::asio::io_context& ioc,
        boost::asio::ssl::context& ssl_ctx
    );

    void connect(
        const std::string& host,
        const std::string& port,
        const std::string& target,
        MessageHandler onMessage,
        ErrorHandler onError
    );

    void send(const std::string& message);
    void sendBinary(const std::vector<uint8_t>& data);
    void close();

private:
    void on_resolve(
        boost::beast::error_code ec,
        boost::asio::ip::tcp::resolver::results_type results
    );
    
    void on_connect(
        boost::beast::error_code ec,
        boost::asio::ip::tcp::resolver::endpoint_type ep
    );
    
    void on_ssl_handshake(boost::beast::error_code ec);
    void on_handshake(boost::beast::error_code ec);
    void on_write(boost::beast::error_code ec, std::size_t bytes_transferred);
    void do_read();
    void on_read(boost::beast::error_code ec, std::size_t bytes_transferred);
    void on_close(boost::beast::error_code ec);

    boost::asio::ip::tcp::resolver resolver_;
    boost::beast::websocket::stream<
        boost::beast::ssl_stream<boost::beast::tcp_stream>> ws_;
    boost::beast::flat_buffer buffer_;
    MessageHandler message_handler_;
    ErrorHandler error_handler_;
    std::string host_;
    std::string target_;
};

} // namespace websocket_client
