#include <string>
#include <iostream>
#include <iomanip>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <array>

using boost::asio::ip::udp;

namespace {

class HelloWorldServer {
public:
    HelloWorldServer(boost::asio::io_service& io_service)
        : _socket(io_service, udp::endpoint(udp::v4(), 50001)),
          _resendFlag(false) // Initialize the flag
    {
        startReceive();
    }

    void setResendFlag(bool flag) {
        _resendFlag = flag;
    }

private:
    void startReceive() {
        _socket.async_receive_from(
            boost::asio::buffer(_recvBuffer), _remoteEndpoint,
            boost::bind(&HelloWorldServer::handleReceive, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    void handleReceive(const boost::system::error_code& error,
                       std::size_t bytes_transferred) {
        if (!error || error == boost::asio::error::message_size) {
            std::cout << "Hello, World\n";
            for (std::size_t i = 0; i < bytes_transferred; ++i) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>( _recvBuffer[i] )<< ' ';
            }
            std::cout << "\n";

            // Check if the resend flag is set
            //if (_resendFlag) {
            //    std::cout <<"Resending..." << std::endl;
            //    auto message = std::make_shared<std::string>(_recvBuffer.data(), bytes_transferred);
            //    _socket.send_to(boost::asio::buffer(*message), _remoteEndpoint);
            //} 
            startReceive();
        }
    }

    void handleSend(std::shared_ptr<std::string> message,
                    const boost::system::error_code& ec,
                    std::size_t bytes_transferred) {
        startReceive(); // Start receiving the next message after sending
    }

    udp::socket _socket;
    udp::endpoint _remoteEndpoint =  udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 50002);
    std::array<uint8_t, 1024> _recvBuffer;
    bool _resendFlag; // Flag to control whether to resend the received message
};

}  // namespace

int main(int argc, const char** argv) {
    try {
        boost::asio::io_service io_service;
        HelloWorldServer server(io_service);

        // Example: Set the resend flag to true
        server.setResendFlag(std::stoi(argv[1]));

        io_service.run();
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }
    return 0;
}
