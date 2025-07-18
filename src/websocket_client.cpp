#include "websocket_client.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/strand.hpp>

namespace websocket_client {

WebSocketClient::WebSocketClient(
    boost::asio::io_context& ioc,
    boost::asio::ssl::context& ssl_ctx
)
    : resolver_(boost::asio::make_strand(ioc))
    , ws_(boost::asio::make_strand(ioc), ssl_ctx)
    , buffer_()
    , host_()
    , target_()
{
}

void WebSocketClient::connect(
    const std::string& host,
    const std::string& port,
    const std::string& target,
    MessageHandler onMessage,
    ErrorHandler onError
)
{
    // Store handlers and connection info
    message_handler_ = std::move(onMessage);
    error_handler_ = std::move(onError);
    host_ = host;
    target_ = target;

    // Set SNI Hostname (many hosts need this to handshake successfully)
    if(!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host.c_str()))
    {
        boost::beast::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
        error_handler_("SSL set hostname failed: " + ec.message());
        return;
    }

    // Look up the domain name
    resolver_.async_resolve(
        host,
        port,
        boost::beast::bind_front_handler(
            &WebSocketClient::on_resolve,
            shared_from_this()));
}

void WebSocketClient::on_resolve(
    boost::beast::error_code ec,
    boost::asio::ip::tcp::resolver::results_type results
)
{
    if(ec)
    {
        error_handler_("Resolve failed: " + ec.message());
        return;
    }

    // Set a timeout on the operation
    boost::beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    boost::beast::get_lowest_layer(ws_).async_connect(
        results,
        boost::beast::bind_front_handler(
            &WebSocketClient::on_connect,
            shared_from_this()));
}

void WebSocketClient::on_connect(
    boost::beast::error_code ec,
    boost::asio::ip::tcp::resolver::endpoint_type ep
)
{
    boost::ignore_unused(ep);
    
    if(ec)
    {
        error_handler_("Connect failed: " + ec.message());
        return;
    }

    // Set a timeout on the operation
    boost::beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    // Perform the SSL handshake
    ws_.next_layer().async_handshake(
        boost::asio::ssl::stream_base::client,
        boost::beast::bind_front_handler(
            &WebSocketClient::on_ssl_handshake,
            shared_from_this()));
}

void WebSocketClient::on_ssl_handshake(boost::beast::error_code ec)
{
    if(ec)
    {
        error_handler_("SSL handshake failed: " + ec.message());
        return;
    }

    // Turn off the timeout on the tcp_stream, because
    // the websocket stream has its own timeout system.
    boost::beast::get_lowest_layer(ws_).expires_never();

    // Set suggested timeout settings for the websocket
    ws_.set_option(
        boost::beast::websocket::stream_base::timeout::suggested(
            boost::beast::role_type::client));

    // Perform the websocket handshake
    ws_.async_handshake(host_, target_,
        boost::beast::bind_front_handler(
            &WebSocketClient::on_handshake,
            shared_from_this()));
}

void WebSocketClient::on_handshake(boost::beast::error_code ec)
{
    if(ec)
    {
        error_handler_("Websocket handshake failed: " + ec.message());
        return;
    }

    // Start reading messages
    do_read();
}

void WebSocketClient::do_read()
{
    ws_.async_read(
        buffer_,
        boost::beast::bind_front_handler(
            &WebSocketClient::on_read,
            shared_from_this()));
}

void WebSocketClient::on_read(
    boost::beast::error_code ec,
    std::size_t bytes_transferred
)
{
    boost::ignore_unused(bytes_transferred);

    if(ec)
    {
        if(ec != boost::beast::websocket::error::closed)
            error_handler_("Read failed: " + ec.message());
        return;
    }

    // Handle the message
    message_handler_(boost::beast::buffers_to_string(buffer_.data()));

    // Clear the buffer
    buffer_.consume(buffer_.size());

    // Queue up another read
    do_read();
}

void WebSocketClient::send(const std::string& message)
{
    ws_.async_write(
        boost::asio::buffer(message),
        boost::beast::bind_front_handler(
            &WebSocketClient::on_write,
            shared_from_this()));
}

void WebSocketClient::sendBinary(const std::vector<uint8_t>& data)
{
    // Set binary mode
    ws_.binary(true);
    
    ws_.async_write(
        boost::asio::buffer(data),
        boost::beast::bind_front_handler(
            &WebSocketClient::on_write,
            shared_from_this()));
}

void WebSocketClient::on_write(
    boost::beast::error_code ec,
    std::size_t bytes_transferred
)
{
    boost::ignore_unused(bytes_transferred);

    if(ec)
    {
        error_handler_("Write failed: " + ec.message());
        return;
    }
    
    // Reset to text mode after any write
    ws_.binary(false);
}

void WebSocketClient::close()
{
    ws_.async_close(boost::beast::websocket::close_code::normal,
        boost::beast::bind_front_handler(
            &WebSocketClient::on_close,
            shared_from_this()));
}

void WebSocketClient::on_close(boost::beast::error_code ec)
{
    if(ec)
    {
        error_handler_("Close failed: " + ec.message());
        return;
    }
}

} // namespace websocket_client
