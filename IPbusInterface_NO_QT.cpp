#include"IPbusInterface_NO_QT.h"
#include <boost/bind.hpp>
#include<iostream>

IPbusTarget::IPbusTarget(boost::asio::io_service & io_service, std::string address, uint16_t lport, uint16_t rport):
m_IPaddress(address), m_localport(lport), m_remoteport(rport),
m_local_endpoint(boost::asio::ip::udp::v4(), lport),
m_remote_endpoint(boost::asio::ip::address::from_string(address), rport),
m_socket(io_service)
{
    std::cerr << "Connecting to " + address << std::endl;
    m_socket.connect(m_remote_endpoint);
    std::cerr << "Connected successfully\n";
    start_async_recv();
}


void IPbusTarget::start_async_recv()
{
    m_socket.async_receive_from(boost::asio::buffer(m_buffer), m_remote_endpoint, 
    boost::bind(&IPbusTarget::handle_recv, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void IPbusTarget::handle_recv(const boost::system::error_code& error, std::size_t bytes_transferred)
{
    if(error) return;
    std::memcpy((char*)&m_packet.response, m_buffer, bytes_transferred);
    std::cout<<"\nMessage recevieved\n!";
}

bool IPbusTarget::reconnect()
{
    m_socket.send_to(boost::asio::buffer((char*)&m_status, sizeof(StatusPacket)), m_remote_endpoint);
}