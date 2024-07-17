#include"IPbusInterface.h"
#include <boost/bind.hpp>
#include<iostream>

IPbusTarget::IPbusTarget(boost::asio::io_context & io_context, std::string address, uint16_t lport, uint16_t rport):
    m_io_context(io_context),
    m_localport(lport), 
    m_remoteport(rport),
    m_IPaddress(address),
    m_local_endpoint(boost::asio::ip::udp::v4(), m_localport),
    m_remote_endpoint(boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(m_IPaddress), m_remoteport)),
    m_socket(io_context),

    m_timer(m_io_context)
{
    open_socket();
    start_timer();
    start_io_thread();
}

IPbusTarget::~IPbusTarget()
{
    stop_timer();
    m_socket.cancel();
    m_socket.close();
    m_thread->join();
}

bool IPbusTarget::open_socket()
{
    std::cerr << "Attempting to open socket..."<< std::endl;
    m_socket.open(boost::asio::ip::udp::v4());

    if(m_socket.is_open())
    {
        std::cerr << "Socket successfully opened" << std::endl;
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


size_t IPbusTarget::sync_recv(char* dest_buffer, size_t max_size) {
    try {
        std::cerr << "Synchronized receiving..." << std::endl;
        size_t bytes_transferred = m_socket.receive_from(boost::asio::buffer(m_buffer, IO_BUFFER_SIZE), m_remote_endpoint);
        std::cout << "Message received: " << bytes_transferred << " bytes" << std::endl;
        std::memcpy(dest_buffer, m_buffer, bytes_transferred);
        return bytes_transferred;
    } catch (const boost::system::system_error& e) {
        std::cerr << "Receive error: " << e.what() << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 0;
    } catch (...) {
        std::cerr << "An unknown error occurred during receive operation." << std::endl;
        return 0;
    }
    return 0;
}

bool IPbusTarget::checkStatus()
{
    std::lock_guard<std::mutex> m_lock(m_transcieve_mutex);

    try {
        std::cerr << "Checking status of device at " << m_IPaddress << std::endl;
        // Send a status packet to the remote endpoint
        m_socket.send_to(boost::asio::buffer(&m_status, sizeof(m_status)), m_remote_endpoint);
        //sync_recv((char*) &m_status_respone, sizeof(m_status_respone));
        std::cerr << "Status check successful: Device is available." << std::endl;
        is_available = true;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to check status: " << e.what() << std::endl;
        is_available = false;
        return false;
    }
}

bool IPbusTarget::transcieve(IPbusControlPacket &p, bool shouldResponseBeProcessed)
{
    if(is_available == false) return false;
    if(p.requestSize <= 1)
    {
        std::cerr << "Empty request" << std::endl;
        return true;
    }
    std::lock_guard<std::mutex> m_lock(m_transcieve_mutex);

    size_t send_bytes =  0;

    try
    {
        send_bytes = m_socket.send_to(boost::asio::buffer((char*)&p.request, p.requestSize * wordSize), m_remote_endpoint);
    }
    catch(const std::exception& e)
    {
        std::cerr<< "Sending packet failed: " << e.what() << std::endl;
        return false;
    }

    if(send_bytes < p.requestSize*wordSize)
    {
        std::cerr << "Sending packer faild: " << send_bytes << " bytes was send instead of " << p.requestSize*wordSize << std::endl;
        return false;
    }

    size_t bytes_recevied  = sync_recv((char*)&p.response, p.requestSize*wordSize);

    if(bytes_recevied == 64 && p.response[0] == m_status.header)
    {
        bytes_recevied = sync_recv((char*)&p.response, p.requestSize*wordSize);
    }

    if(bytes_recevied == 0)
    {
        std::cerr << "Empty response from " << m_IPaddress << std::endl;
        return false;
    }
    else if (bytes_recevied / wordSize > p.responseSize || p.response[0] != p.request[0] || bytes_recevied % wordSize > 0) {
            std::cerr << "Incorrect response: received " << bytes_recevied << " bytes instead of " << p.responseSize*wordSize << std::endl;
            return false;
    } else {
            p.responseSize = uint16_t(bytes_recevied / wordSize); //response can be shorter then expected if a transaction wasn't successful
            bool result = shouldResponseBeProcessed ? p.processResponse() : true;
            p.reset();
            return result;
    }
}

void IPbusTarget::reset_timer()
{
    std::lock_guard<std::mutex> lock(m_timer_mutex);
    if(m_stop_timer)
    {   
        m_timer.cancel();
        return;
    }
    m_timer.expires_from_now(m_tick);
    m_timer.async_wait(boost::bind(&IPbusTarget::sync, this, boost::asio::placeholders::error));
}

void IPbusTarget::sync(const boost::system::error_code& error)
{
    checkStatus();
    reset_timer();
}

void IPbusTarget::io_context_run()
{
    m_io_context.run();
}

void IPbusTarget::start_io_thread()
{
    m_thread = std::make_shared<std::thread>([this](){this->io_context_run();});
}

void IPbusTarget::stop_io_thread()
{
    m_thread->~thread();
}

void IPbusTarget::start_timer()
{
    std::lock_guard<std::mutex> lock(m_timer_mutex);
    m_stop_timer = false;
    m_timer.expires_from_now(m_tick);
    m_timer.async_wait(boost::bind(&IPbusTarget::sync, this, boost::asio::placeholders::error));
}

void IPbusTarget::stop_timer()
{
    std::lock_guard<std::mutex> lock(m_timer_mutex); 
    m_stop_timer = true;
}