#include"IPbusInterface.h"
#include <boost/bind.hpp>
#include<iostream>

IPbusTarget::IPbusTarget(boost::asio::io_context & io_context, std::string address, uint16_t lport, uint16_t rport):
    m_io_context(io_context),
    m_IPaddress(address), 
    m_localport(lport), 
    m_remoteport(rport),
    m_socket(io_context)
{
    open_socket();
    checkStatus();
}

bool IPbusTarget::open_socket()
{
    boost::asio::ip::udp::resolver resolver(m_io_context);
    m_remote_endpoint = *resolver.resolve(boost::asio::ip::udp::v4(), m_IPaddress, std::to_string(m_remoteport)).begin();

    std::cerr << "Attempting to open socket..."<< std::endl;
    m_socket.open(boost::asio::ip::udp::v4());

    if(m_socket.is_open())
    {
        std::cerr << "Socket successfully opened" << std::endl;
        start_async_recv();
        return true;
    }
    else
    {
        std::cerr << "Failed to open socket."  << std::endl;
        return false;
    }
}

bool IPbusTarget::reopen()
{
   if (m_socket.is_open()) {
        std::cerr << "Socket is already open." << std::endl;
        return true;
    }
    std::cerr << "Socket is not open. Attempting to reopen..." << std::endl;
    return open_socket();  // Reopen the socket if it is not open
}


void IPbusTarget::start_async_recv()
{
    m_socket.async_receive_from(boost::asio::buffer(m_buffer, IO_BUFFER_SIZE), m_remote_endpoint, 
    boost::bind(&IPbusTarget::handle_recv, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void IPbusTarget::handle_recv(const boost::system::error_code& error, std::size_t bytes_transferred)
{
    if (error) {
        std::cerr << "Error receiving data: " << error.message() << std::endl;
        return;
    }
    std::memcpy((char*)&m_packet.response, m_buffer, bytes_transferred);
    std::cout << "Message received: " <<  bytes_transferred << " bytes" << std::endl;
    start_async_recv();  // Start receiving the next message
}

bool IPbusTarget::checkStatus()
{
    try {
        std::cerr << "Checking status of device at " << m_IPaddress << std::endl;

        if (!m_socket.is_open()) {
            std::cerr << "Socket is not open. Attempting to open socket..." << std::endl;
            m_socket.open(boost::asio::ip::udp::v4());
        }

        // Send a status packet to the remote endpoint
        m_socket.send_to(boost::asio::buffer(&m_status, sizeof(m_status)), m_remote_endpoint);
        std::cout << "Status packet has been sent" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to check status: " << e.what() << std::endl;
        return false;
    }
}